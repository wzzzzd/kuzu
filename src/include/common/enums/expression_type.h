#pragma once

#include <cstdint>
#include <string>

namespace kuzu {
namespace common {

/**
 * Function name is a temporary identifier used for binder because grammar does not parse built in
 * functions. After binding, expression type should replace function name and used as identifier.
 */
// aggregate
const char* const COUNT_STAR_FUNC_NAME = "COUNT_STAR";
const char* const COUNT_FUNC_NAME = "COUNT";
const char* const SUM_FUNC_NAME = "SUM";
const char* const AVG_FUNC_NAME = "AVG";
const char* const MIN_FUNC_NAME = "MIN";
const char* const MAX_FUNC_NAME = "MAX";
const char* const COLLECT_FUNC_NAME = "COLLECT";

// string
const char* const LENGTH_FUNC_NAME = "LENGTH";

// Node/Rel functions.
const char* const ID_FUNC_NAME = "ID";
const char* const LABEL_FUNC_NAME = "LABEL";
const char* const OFFSET_FUNC_NAME = "OFFSET";
const char* const START_NODE_FUNC_NAME = "START_NODE";
const char* const END_NODE_FUNC_NAME = "END_NODE";

// Path functions
const char* const NODES_FUNC_NAME = "NODES";
const char* const RELS_FUNC_NAME = "RELS";
const char* const PROPERTIES_FUNC_NAME = "PROPERTIES";
const char* const IS_TRAIL_FUNC_NAME = "IS_TRAIL";
const char* const IS_ACYCLIC_FUNC_NAME = "IS_ACYCLIC";

// RDF functions
const char* const TYPE_FUNC_NAME = "TYPE";
const char* const VALIDATE_PREDICATE_FUNC_NAME = "VALIDATE_PREDICATE";

// Table functions
const char* const TABLE_INFO_FUNC_NAME = "TABLE_INFO";
const char* const DB_VERSION_FUNC_NAME = "DB_VERSION";
const char* const CURRENT_SETTING_FUNC_NAME = "CURRENT_SETTING";
const char* const SHOW_TABLES_FUNC_NAME = "SHOW_TABLES";
const char* const SHOW_CONNECTION_FUNC_NAME = "SHOW_CONNECTION";
const char* const STORAGE_INFO_FUNC_NAME = "STORAGE_INFO";
// Table functions - read functions
const char* const READ_PARQUET_FUNC_NAME = "READ_PARQUET";
const char* const READ_NPY_FUNC_NAME = "READ_NPY";
const char* const READ_CSV_SERIAL_FUNC_NAME = "READ_CSV_SERIAL";
const char* const READ_CSV_PARALLEL_FUNC_NAME = "READ_CSV_PARALLEL";
const char* const READ_RDF_RESOURCE_FUNC_NAME = "READ_RDF_RESOURCE";
const char* const READ_RDF_LITERAL_FUNC_NAME = "READ_RDF_LITERAL";
const char* const READ_RDF_RESOURCE_TRIPLE_FUNC_NAME = "READ_RDF_RESOURCE_TRIPLE";
const char* const READ_RDF_LITERAL_TRIPLE_FUNC_NAME = "READ_RDF_LITERAL_TRIPLE";
const char* const READ_RDF_ALL_TRIPLE_FUNC_NAME = "READ_RDF_ALL_TRIPLE";
const char* const IN_MEM_READ_RDF_RESOURCE_FUNC_NAME = "IN_MEM_READ_RDF_RESOURCE";
const char* const IN_MEM_READ_RDF_LITERAL_FUNC_NAME = "IN_MEM_READ_RDF_LITERAL";
const char* const IN_MEM_READ_RDF_RESOURCE_TRIPLE_FUNC_NAME = "IN_MEM_READ_RDF_RESOURCE_TRIPLE";
const char* const IN_MEM_READ_RDF_LITERAL_TRIPLE_FUNC_NAME = "IN_MEM_READ_RDF_LITERAL_TRIPLE";
const char* const READ_PANDAS_FUNC_NAME = "READ_PANDAS";
const char* const READ_PYARROW_FUNC_NAME = "READ_PYARROW";
const char* const READ_FTABLE_FUNC_NAME = "READ_FTABLE";

enum class ExpressionType : uint8_t {

    // Boolean Connection Expressions
    OR = 0,
    XOR = 1,
    AND = 2,
    NOT = 3,

    // Comparison Expressions
    EQUALS = 10,
    NOT_EQUALS = 11,
    GREATER_THAN = 12,
    GREATER_THAN_EQUALS = 13,
    LESS_THAN = 14,
    LESS_THAN_EQUALS = 15,

    // Null Operator Expressions
    IS_NULL = 50,
    IS_NOT_NULL = 51,

    PROPERTY = 60,

    LITERAL = 70,

    STAR = 80,

    VARIABLE = 90,
    PATH = 91,
    PATTERN = 92, // Node & Rel pattern

    PARAMETER = 100,

    FUNCTION = 110,

    AGGREGATE_FUNCTION = 130,

    SUBQUERY = 190,

    CASE_ELSE = 200,

    MACRO = 210,
};

bool isExpressionUnary(ExpressionType type);
bool isExpressionBinary(ExpressionType type);
bool isExpressionBoolConnection(ExpressionType type);
bool isExpressionComparison(ExpressionType type);
bool isExpressionNullOperator(ExpressionType type);
bool isExpressionLiteral(ExpressionType type);
bool isExpressionAggregate(ExpressionType type);
bool isExpressionSubquery(ExpressionType type);

std::string expressionTypeToString(ExpressionType type);

} // namespace common
} // namespace kuzu
