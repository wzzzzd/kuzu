#include "function/cast/vector_cast_functions.h"

#include "function/cast/cast_functions.h"

using namespace kuzu::common;

namespace kuzu {
namespace function {

bool VectorCastFunction::hasImplicitCast(const LogicalType& srcType, const LogicalType& dstType) {
    // We allow cast between any numerical types
    if (LogicalTypeUtils::isNumerical(srcType) && LogicalTypeUtils::isNumerical(dstType)) {
        return true;
    }
    switch (srcType.getLogicalTypeID()) {
    case LogicalTypeID::DATE: {
        switch (dstType.getLogicalTypeID()) {
        case LogicalTypeID::TIMESTAMP:
            return true;
        default:
            return false;
        }
    }
    case LogicalTypeID::STRING: {
        switch (dstType.getLogicalTypeID()) {
        case LogicalTypeID::DATE:
        case LogicalTypeID::TIMESTAMP:
        case LogicalTypeID::INTERVAL:
            return true;
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

std::string VectorCastFunction::bindImplicitCastFuncName(const LogicalType& dstType) {
    switch (dstType.getLogicalTypeID()) {
    case LogicalTypeID::SERIAL:
        return CAST_TO_SERIAL_FUNC_NAME;
    case LogicalTypeID::INT64:
        return CAST_TO_INT64_FUNC_NAME;
    case LogicalTypeID::INT32:
        return CAST_TO_INT32_FUNC_NAME;
    case LogicalTypeID::INT16:
        return CAST_TO_INT16_FUNC_NAME;
    case LogicalTypeID::INT8:
        return CAST_TO_INT8_FUNC_NAME;
    case LogicalTypeID::UINT64:
        return CAST_TO_UINT64_FUNC_NAME;
    case LogicalTypeID::UINT32:
        return CAST_TO_UINT32_FUNC_NAME;
    case LogicalTypeID::UINT16:
        return CAST_TO_UINT16_FUNC_NAME;
    case LogicalTypeID::UINT8:
        return CAST_TO_UINT8_FUNC_NAME;
    case LogicalTypeID::INT128:
        return CAST_TO_INT128_FUNC_NAME;
    case LogicalTypeID::FLOAT:
        return CAST_TO_FLOAT_FUNC_NAME;
    case LogicalTypeID::DOUBLE:
        return CAST_TO_DOUBLE_FUNC_NAME;
    case LogicalTypeID::DATE:
        return CAST_TO_DATE_FUNC_NAME;
    case LogicalTypeID::TIMESTAMP:
        return CAST_TO_TIMESTAMP_FUNC_NAME;
    case LogicalTypeID::INTERVAL:
        return CAST_TO_INTERVAL_FUNC_NAME;
    case LogicalTypeID::STRING:
        return CAST_TO_STRING_FUNC_NAME;
    default:
        throw NotImplementedException("bindImplicitCastFuncName()");
    }
}

void VectorCastFunction::bindImplicitCastFunc(
    LogicalTypeID sourceTypeID, LogicalTypeID targetTypeID, scalar_exec_func& func) {
    switch (targetTypeID) {
    case LogicalTypeID::INT8: {
        bindImplicitNumericalCastFunc<int8_t, CastToInt8>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::INT16: {
        bindImplicitNumericalCastFunc<int16_t, CastToInt16>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::INT32: {
        bindImplicitNumericalCastFunc<int32_t, CastToInt32>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::SERIAL:
    case LogicalTypeID::INT64: {
        bindImplicitNumericalCastFunc<int64_t, CastToInt64>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT8: {
        bindImplicitNumericalCastFunc<uint8_t, CastToUInt8>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT16: {
        bindImplicitNumericalCastFunc<uint16_t, CastToUInt16>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT32: {
        bindImplicitNumericalCastFunc<uint32_t, CastToUInt32>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::UINT64: {
        bindImplicitNumericalCastFunc<uint64_t, CastToUInt64>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::INT128: {
        bindImplicitNumericalCastFunc<int128_t, CastToInt128>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::FLOAT: {
        bindImplicitNumericalCastFunc<float_t, CastToFloat>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::DOUBLE: {
        bindImplicitNumericalCastFunc<double_t, CastToDouble>(sourceTypeID, func);
        return;
    }
    case LogicalTypeID::DATE: {
        assert(sourceTypeID == LogicalTypeID::STRING);
        func = &UnaryExecFunction<ku_string_t, date_t, CastStringToDate>;
        return;
    }
    case LogicalTypeID::TIMESTAMP: {
        assert(sourceTypeID == LogicalTypeID::STRING || sourceTypeID == LogicalTypeID::DATE);
        func = sourceTypeID == LogicalTypeID::STRING ?
                   &UnaryExecFunction<ku_string_t, timestamp_t, CastStringToTimestamp> :
                   &UnaryExecFunction<date_t, timestamp_t, CastDateToTimestamp>;
        return;
    }
    case LogicalTypeID::INTERVAL: {
        assert(sourceTypeID == LogicalTypeID::STRING);
        func = &UnaryExecFunction<ku_string_t, interval_t, CastStringToInterval>;
        return;
    }
    default:
        throw NotImplementedException("Unimplemented casting operation from " +
                                      LogicalTypeUtils::dataTypeToString(sourceTypeID) + " to " +
                                      LogicalTypeUtils::dataTypeToString(targetTypeID) + ".");
    }
}

vector_function_definitions CastToDateVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_DATE_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::DATE,
        UnaryExecFunction<ku_string_t, date_t, CastStringToDate>));
    return result;
}

vector_function_definitions CastToTimestampVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(std::make_unique<VectorFunctionDefinition>(CAST_TO_TIMESTAMP_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::TIMESTAMP,
        UnaryExecFunction<ku_string_t, timestamp_t, CastStringToTimestamp>));
    return result;
}

vector_function_definitions CastToIntervalVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_INTERVAL_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::INTERVAL,
        UnaryExecFunction<ku_string_t, interval_t, CastStringToInterval>));
    return result;
}

void CastToStringVectorFunction::getUnaryCastExecFunction(
    common::LogicalTypeID typeID, scalar_exec_func& func) {
    switch (typeID) {
    case common::LogicalTypeID::BOOL: {
        func = UnaryCastExecFunction<bool, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::SERIAL:
    case common::LogicalTypeID::INT64: {
        func = UnaryCastExecFunction<int64_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::INT32: {
        func = UnaryCastExecFunction<int32_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::INT16: {
        func = UnaryCastExecFunction<int16_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::INT8: {
        func = UnaryCastExecFunction<int8_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::UINT64: {
        func = UnaryCastExecFunction<uint64_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::UINT32: {
        func = UnaryCastExecFunction<uint32_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::UINT16: {
        func = UnaryCastExecFunction<uint16_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::INT128: {
        func = UnaryCastExecFunction<int128_t, ku_string_t, CastToString>;
    }
    case common::LogicalTypeID::UINT8: {
        func = UnaryCastExecFunction<uint8_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::DOUBLE: {
        func = UnaryCastExecFunction<double_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::FLOAT: {
        func = UnaryCastExecFunction<float_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::DATE: {
        func = UnaryCastExecFunction<date_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::TIMESTAMP: {
        func = UnaryCastExecFunction<timestamp_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::INTERVAL: {
        func = UnaryCastExecFunction<interval_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::INTERNAL_ID: {
        func = UnaryCastExecFunction<internalID_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::BLOB:
    case common::LogicalTypeID::STRING: {
        func = UnaryCastExecFunction<ku_string_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::VAR_LIST: {
        func = UnaryCastExecFunction<list_entry_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::MAP: {
        func = UnaryCastExecFunction<map_entry_t, ku_string_t, CastToString>;
    } break;
    case common::LogicalTypeID::NODE:
    case common::LogicalTypeID::REL:
    case common::LogicalTypeID::STRUCT: {
        func = UnaryCastExecFunction<struct_entry_t, ku_string_t, CastToString>;
    } break;
        // LCOV_EXCL_START
    default:
        throw common::NotImplementedException{
            "CastToStringVectorFunction::getUnaryCastExecFunction"};
        // LCOV_EXCL_END
    }
}

vector_function_definitions CastToStringVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.reserve(LogicalTypeUtils::getAllValidLogicTypes().size());
    for (auto& type : LogicalTypeUtils::getAllValidLogicTypes()) {
        scalar_exec_func execFunc;
        getUnaryCastExecFunction(type.getLogicalTypeID(), execFunc);
        auto definition = std::make_unique<VectorFunctionDefinition>(CAST_TO_STRING_FUNC_NAME,
            std::vector<LogicalTypeID>{type.getLogicalTypeID()}, LogicalTypeID::STRING, execFunc);
        result.push_back(std::move(definition));
    }
    return result;
}

vector_function_definitions CastToBlobVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_BLOB_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::BLOB,
        UnaryCastExecFunction<ku_string_t, blob_t, CastToBlob>));
    return result;
}

vector_function_definitions CastToBoolVectorFunction::getDefinitions() {
    vector_function_definitions result;
    result.push_back(make_unique<VectorFunctionDefinition>(CAST_TO_BOOL_FUNC_NAME,
        std::vector<LogicalTypeID>{LogicalTypeID::STRING}, LogicalTypeID::BOOL,
        UnaryExecFunction<ku_string_t, bool, CastToBool>));
    return result;
}

vector_function_definitions CastToDoubleVectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<double_t, CastToDouble>(
            CAST_TO_DOUBLE_FUNC_NAME, typeID, LogicalTypeID::DOUBLE));
    }
    result.push_back(bindVectorFunction<double_t, CastToDouble>(
        CAST_TO_DOUBLE_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::DOUBLE));
    return result;
}

vector_function_definitions CastToFloatVectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<float_t, CastToFloat>(
            CAST_TO_FLOAT_FUNC_NAME, typeID, LogicalTypeID::FLOAT));
    }
    result.push_back(bindVectorFunction<float_t, CastToFloat>(
        CAST_TO_FLOAT_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::FLOAT));
    return result;
}

vector_function_definitions CastToSerialVectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<int64_t, CastToSerial>(
            CAST_TO_SERIAL_FUNC_NAME, typeID, LogicalTypeID::SERIAL));
    }
    result.push_back(bindVectorFunction<int64_t, CastToSerial>(
        CAST_TO_SERIAL_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::SERIAL));
    return result;
}

vector_function_definitions CastToInt64VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<int64_t, CastToInt64>(
            CAST_TO_INT64_FUNC_NAME, typeID, LogicalTypeID::INT64));
    }
    result.push_back(bindVectorFunction<int64_t, CastToInt64>(
        CAST_TO_INT64_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT64));
    return result;
}

vector_function_definitions CastToInt32VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<int32_t, CastToInt32>(
            CAST_TO_INT32_FUNC_NAME, typeID, LogicalTypeID::INT32));
    }
    result.push_back(bindVectorFunction<int32_t, CastToInt32>(
        CAST_TO_INT32_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT32));
    return result;
}

vector_function_definitions CastToInt16VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<int16_t, CastToInt16>(
            CAST_TO_INT16_FUNC_NAME, typeID, LogicalTypeID::INT16));
    }
    result.push_back(bindVectorFunction<int16_t, CastToInt16>(
        CAST_TO_INT16_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT16));
    return result;
}

vector_function_definitions CastToInt8VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<int8_t, CastToInt8>(
            CAST_TO_INT8_FUNC_NAME, typeID, LogicalTypeID::INT8));
    }
    result.push_back(bindVectorFunction<int8_t, CastToInt8>(
        CAST_TO_INT8_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT8));
    return result;
}

vector_function_definitions CastToUInt64VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<uint64_t, CastToUInt64>(
            CAST_TO_UINT64_FUNC_NAME, typeID, LogicalTypeID::UINT64));
    }
    result.push_back(bindVectorFunction<uint64_t, CastToUInt64>(
        CAST_TO_UINT64_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT64));
    return result;
}

vector_function_definitions CastToUInt32VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<uint32_t, CastToUInt32>(
            CAST_TO_UINT32_FUNC_NAME, typeID, LogicalTypeID::UINT32));
    }
    result.push_back(bindVectorFunction<uint32_t, CastToUInt32>(
        CAST_TO_UINT32_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT32));
    return result;
}

vector_function_definitions CastToUInt16VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<uint16_t, CastToUInt16>(
            CAST_TO_UINT16_FUNC_NAME, typeID, LogicalTypeID::UINT16));
    }
    result.push_back(bindVectorFunction<uint16_t, CastToUInt16>(
        CAST_TO_UINT16_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT16));
    return result;
}

vector_function_definitions CastToUInt8VectorFunction::getDefinitions() {
    vector_function_definitions result;
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<uint8_t, CastToUInt8>(
            CAST_TO_UINT8_FUNC_NAME, typeID, LogicalTypeID::UINT8));
    }
    result.push_back(bindVectorFunction<uint8_t, CastToUInt8>(
        CAST_TO_UINT8_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::UINT8));
    return result;
}

vector_function_definitions CastToInt128VectorFunction::getDefinitions() {
    vector_function_definitions result;
    // down cast
    for (auto typeID : LogicalTypeUtils::getNumericalLogicalTypeIDs()) {
        result.push_back(bindVectorFunction<int128_t, CastToInt128>(
            CAST_TO_INT128_FUNC_NAME, typeID, LogicalTypeID::INT128));
    }
    result.push_back(bindVectorFunction<int128_t, CastToInt128>(
        CAST_TO_INT128_FUNC_NAME, LogicalTypeID::STRING, LogicalTypeID::INT128));
    return result;
}

} // namespace function
} // namespace kuzu
