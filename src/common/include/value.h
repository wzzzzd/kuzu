#pragma once

#include <cstdint>
#include <string>

#include "src/common/include/gf_string.h"
#include "src/common/include/types.h"

using namespace std;

namespace graphflow {
namespace common {

class Value {
public:
    Value(const Value& value) { *this = value; }

    explicit Value(DataType dataType) : dataType(dataType) {}

    explicit Value(uint8_t value) : dataType(BOOL) { this->val.booleanVal = value; }

    explicit Value(int32_t value) : dataType(INT32) { this->val.int32Val = value; }

    explicit Value(double value) : dataType(DOUBLE) { this->val.doubleVal = value; }

    explicit Value(const string& value) : dataType(STRING) { this->val.strVal.set(value); }

    Value& operator=(const Value& other);

    // todo: move this to the StringVector level
    void castToString();

    string toString() const;

public:
    union Val {
        uint8_t booleanVal;
        int32_t int32Val;
        double doubleVal;
        gf_string_t strVal{};
        nodeID_t nodeID;
    } val;

    DataType dataType;
};

} // namespace common
} // namespace graphflow