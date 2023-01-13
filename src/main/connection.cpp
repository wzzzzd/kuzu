#include "main/connection.h"

#include "main/database.h"
#include "main/plan_printer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "processor/mapper/plan_mapper.h"

using namespace kuzu::parser;
using namespace kuzu::binder;
using namespace kuzu::planner;
using namespace kuzu::processor;
using namespace kuzu::transaction;

namespace kuzu {
namespace main {

Connection::Connection(Database* database) {
    assert(database != nullptr);
    this->database = database;
    clientContext = make_unique<ClientContext>();
    transactionMode = AUTO_COMMIT;
}

Connection::~Connection() {
    if (activeTransaction) {
        database->transactionManager->rollback(activeTransaction.get());
    }
}

unique_ptr<QueryResult> Connection::query(const string& query) {
    lock_t lck{mtx};
    unique_ptr<PreparedStatement> preparedStatement;
    preparedStatement = prepareNoLock(query);
    return executeAndAutoCommitIfNecessaryNoLock(preparedStatement.get());
}

unique_ptr<QueryResult> Connection::queryResultWithError(std::string& errMsg) {
    auto queryResult = make_unique<QueryResult>();
    queryResult->success = false;
    queryResult->errMsg = errMsg;
    return queryResult;
}

std::unique_ptr<PreparedStatement> Connection::prepareNoLock(
    const string& query, bool enumerateAllPlans) {
    auto preparedStatement = make_unique<PreparedStatement>();
    if (query.empty()) {
        preparedStatement->success = false;
        preparedStatement->errMsg = "Connection Exception: Query is empty.";
        return preparedStatement;
    }
    auto compilingTimer = TimeMetric(true /* enable */);
    compilingTimer.start();
    unique_ptr<ExecutionContext> executionContext;
    unique_ptr<LogicalPlan> logicalPlan;
    try {
        // parsing
        auto statement = Parser::parseQuery(query);
        preparedStatement->preparedSummary.isExplain = statement->isExplain();
        preparedStatement->preparedSummary.isProfile = statement->isProfile();
        // binding
        auto binder = Binder(*database->catalog);
        auto boundStatement = binder.bind(*statement);
        preparedStatement->statementType = boundStatement->getStatementType();
        preparedStatement->readOnly = boundStatement->isReadOnly();
        preparedStatement->parameterMap = binder.getParameterMap();
        preparedStatement->statementResult = boundStatement->getStatementResult()->copy();
        // planning
        auto& nodeStatistics =
            database->storageManager->getNodesStore().getNodesStatisticsAndDeletedIDs();
        auto& relStatistics = database->storageManager->getRelsStore().getRelsStatistics();
        if (enumerateAllPlans) {
            preparedStatement->logicalPlans = Planner::getAllPlans(
                *database->catalog, nodeStatistics, relStatistics, *boundStatement);
        } else {
            preparedStatement->logicalPlans.push_back(Planner::getBestPlan(
                *database->catalog, nodeStatistics, relStatistics, *boundStatement));
        }
    } catch (exception& exception) {
        preparedStatement->success = false;
        preparedStatement->errMsg = exception.what();
    }
    compilingTimer.stop();
    preparedStatement->preparedSummary.compilingTime = compilingTimer.getElapsedTimeMS();
    return preparedStatement;
}

string Connection::getNodeTableNames() {
    lock_t lck{mtx};
    string result = "Node tables: \n";
    for (auto& tableIDSchema : database->catalog->getReadOnlyVersion()->getNodeTableSchemas()) {
        result += "\t" + tableIDSchema.second->tableName + "\n";
    }
    return result;
}

string Connection::getRelTableNames() {
    lock_t lck{mtx};
    string result = "Rel tables: \n";
    for (auto& tableIDSchema : database->catalog->getReadOnlyVersion()->getRelTableSchemas()) {
        result += "\t" + tableIDSchema.second->tableName + "\n";
    }
    return result;
}

string Connection::getNodePropertyNames(const string& tableName) {
    lock_t lck{mtx};
    auto catalog = database->catalog.get();
    if (!catalog->getReadOnlyVersion()->containNodeTable(tableName)) {
        throw Exception("Cannot find node table " + tableName);
    }
    string result = tableName + " properties: \n";
    auto tableID = catalog->getReadOnlyVersion()->getNodeTableIDFromName(tableName);
    auto primaryKeyPropertyID =
        catalog->getReadOnlyVersion()->getNodeTableSchema(tableID)->getPrimaryKey().propertyID;
    for (auto& property : catalog->getReadOnlyVersion()->getAllNodeProperties(tableID)) {
        result += "\t" + property.name + " " + Types::dataTypeToString(property.dataType);
        result += property.propertyID == primaryKeyPropertyID ? "(PRIMARY KEY)\n" : "\n";
    }
    return result;
}

string Connection::getRelPropertyNames(const string& relTableName) {
    lock_t lck{mtx};
    auto catalog = database->catalog.get();
    if (!catalog->getReadOnlyVersion()->containRelTable(relTableName)) {
        throw Exception("Cannot find rel table " + relTableName);
    }
    auto relTableID = catalog->getReadOnlyVersion()->getRelTableIDFromName(relTableName);
    string result = relTableName + " src nodes: \n";
    for (auto& nodeTableID :
        catalog->getReadOnlyVersion()->getNodeTableIDsForRelTableDirection(relTableID, FWD)) {
        result += "\t" + catalog->getReadOnlyVersion()->getTableName(nodeTableID) + "\n";
    }
    result += relTableName + " dst nodes: \n";
    for (auto& nodeTableID :
        catalog->getReadOnlyVersion()->getNodeTableIDsForRelTableDirection(relTableID, BWD)) {
        result += "\t" + catalog->getReadOnlyVersion()->getTableName(nodeTableID) + "\n";
    }
    result += relTableName + " properties: \n";
    for (auto& property : catalog->getReadOnlyVersion()->getRelProperties(relTableID)) {
        if (TableSchema::isReservedPropertyName(property.name)) {
            continue;
        }
        result += "\t" + property.name + " " + Types::dataTypeToString(property.dataType) + "\n";
    }
    return result;
}

std::unique_ptr<QueryResult> Connection::executeWithParams(
    PreparedStatement* preparedStatement, unordered_map<string, shared_ptr<Value>>& inputParams) {
    lock_t lck{mtx};
    if (!preparedStatement->isSuccess()) {
        return queryResultWithError(preparedStatement->errMsg);
    }
    try {
        bindParametersNoLock(preparedStatement, inputParams);
    } catch (Exception& exception) {
        string errMsg = exception.what();
        return queryResultWithError(errMsg);
    }
    return executeAndAutoCommitIfNecessaryNoLock(preparedStatement);
}

void Connection::bindParametersNoLock(
    PreparedStatement* preparedStatement, unordered_map<string, shared_ptr<Value>>& inputParams) {
    auto& parameterMap = preparedStatement->parameterMap;
    for (auto& [name, value] : inputParams) {
        if (!parameterMap.contains(name)) {
            throw Exception("Parameter " + name + " not found.");
        }
        auto expectParam = parameterMap.at(name);
        if (expectParam->dataType != value->getDataType()) {
            throw Exception("Parameter " + name + " has data type " +
                            Types::dataTypeToString(value->getDataType()) + " but expect " +
                            Types::dataTypeToString(expectParam->dataType) + ".");
        }
        parameterMap.at(name)->copyValueFrom(*value);
    }
}

std::unique_ptr<QueryResult> Connection::executeAndAutoCommitIfNecessaryNoLock(
    PreparedStatement* preparedStatement, uint32_t planIdx) {
    auto mapper = PlanMapper(
        *database->storageManager, database->memoryManager.get(), database->catalog.get());
    unique_ptr<PhysicalPlan> physicalPlan;
    if (preparedStatement->isSuccess()) {
        try {
            physicalPlan =
                mapper.mapLogicalPlanToPhysical(preparedStatement->logicalPlans[planIdx].get(),
                    preparedStatement->getExpressionsToCollect(), preparedStatement->statementType);
        } catch (exception& exception) {
            preparedStatement->success = false;
            preparedStatement->errMsg = exception.what();
        }
    }
    if (!preparedStatement->isSuccess()) {
        rollbackIfNecessaryNoLock();
        return queryResultWithError(preparedStatement->errMsg);
    }
    auto queryResult = make_unique<QueryResult>(preparedStatement->preparedSummary);
    auto profiler = make_unique<Profiler>();
    auto executionContext = make_unique<ExecutionContext>(clientContext->numThreadsForExecution,
        profiler.get(), database->memoryManager.get(), database->bufferManager.get());
    // Execute query if EXPLAIN is not enabled.
    if (!preparedStatement->preparedSummary.isExplain) {
        profiler->enabled = preparedStatement->preparedSummary.isProfile;
        auto executingTimer = TimeMetric(true /* enable */);
        executingTimer.start();
        shared_ptr<FactorizedTable> resultFT;
        try {
            beginTransactionIfAutoCommit(preparedStatement);
            executionContext->transaction = activeTransaction.get();
            resultFT =
                database->queryProcessor->execute(physicalPlan.get(), executionContext.get());
            if (AUTO_COMMIT == transactionMode) {
                commitNoLock();
            }
        } catch (Exception& exception) {
            rollbackIfNecessaryNoLock();
            string errMsg = exception.what();
            return queryResultWithError(errMsg);
        }
        executingTimer.stop();
        queryResult->querySummary->executionTime = executingTimer.getElapsedTimeMS();
        queryResult->initResultTableAndIterator(std::move(resultFT),
            preparedStatement->statementResult->getColumns(),
            preparedStatement->statementResult->getExpressionsToCollectPerColumn());
    }
    auto planPrinter = make_unique<PlanPrinter>(physicalPlan.get(), std::move(profiler));
    queryResult->querySummary->planInJson = planPrinter->printPlanToJson();
    queryResult->querySummary->planInOstream = planPrinter->printPlanToOstream();
    return queryResult;
}

void Connection::beginTransactionNoLock(TransactionType type) {
    if (activeTransaction) {
        throw ConnectionException(
            "Connection already has an active transaction. Applications can have one "
            "transaction per connection at any point in time. For concurrent multiple "
            "transactions, please open other connections. Current active transaction is not "
            "affected by this exception and can still be used.");
    }
    activeTransaction = type == READ_ONLY ?
                            database->transactionManager->beginReadOnlyTransaction() :
                            database->transactionManager->beginWriteTransaction();
}

void Connection::commitOrRollbackNoLock(bool isCommit, bool skipCheckpointForTesting) {
    if (activeTransaction) {
        if (activeTransaction->isWriteTransaction()) {
            database->commitAndCheckpointOrRollback(
                activeTransaction.get(), isCommit, skipCheckpointForTesting);
        } else {
            isCommit ? database->transactionManager->commit(activeTransaction.get()) :
                       database->transactionManager->rollback(activeTransaction.get());
        }
        activeTransaction.reset();
        transactionMode = AUTO_COMMIT;
    }
}

void Connection::beginTransactionIfAutoCommit(PreparedStatement* preparedStatement) {
    if (!preparedStatement->isReadOnly() && activeTransaction && activeTransaction->isReadOnly()) {
        throw ConnectionException("Can't execute a write query inside a read-only transaction.");
    }
    if (!preparedStatement->allowActiveTransaction() && activeTransaction) {
        throw ConnectionException(
            "DDL and CopyCSV statements are automatically wrapped in a "
            "transaction and committed. As such, they cannot be part of an "
            "active transaction, please commit or rollback your previous transaction and "
            "issue a ddl query without opening a transaction.");
    }
    if (AUTO_COMMIT == transactionMode) {
        assert(!activeTransaction);
        // If the caller didn't explicitly start a transaction, we do so now and commit or
        // rollback here if necessary, i.e., if the given prepared statement has write
        // operations.
        beginTransactionNoLock(preparedStatement->isReadOnly() ? READ_ONLY : WRITE);
    }
    if (!activeTransaction) {
        assert(MANUAL == transactionMode);
        throw ConnectionException(
            "Transaction mode is manual but there is no active transaction. Please begin a "
            "transaction or set the transaction mode of the connection to AUTO_COMMIT");
    }
}

} // namespace main
} // namespace kuzu
