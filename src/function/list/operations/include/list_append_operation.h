#pragma once

#include <cassert>
#include <cstring>

#include "src/common/types/include/gf_list.h"

using namespace std;
using namespace graphflow::common;

namespace graphflow {
namespace function {
namespace operation {

struct ListAppend {
    template<typename T>
    static inline void operation(gf_list_t& list, T& element, gf_list_t& result, bool isListNull,
        bool isElementNull, ValueVector& resultValueVector) {
        assert(!isListNull && !isElementNull);
        auto elementSize = Types::getDataTypeSize(*resultValueVector.dataType.childType);
        result.overflowPtr = reinterpret_cast<uint64_t>(
            resultValueVector.getOverflowBuffer().allocateSpace((list.size + 1) * elementSize));
        result.size = list.size + 1;
        gf_list_t tmpList;
        TypeUtils::copyListRecursiveIfNested(
            list, tmpList, resultValueVector.dataType, resultValueVector.getOverflowBuffer());
        memcpy(reinterpret_cast<uint8_t*>(result.overflowPtr),
            reinterpret_cast<uint8_t*>(tmpList.overflowPtr), list.size * elementSize);
        TypeUtils::setListElement(result, list.size, element, resultValueVector.dataType,
            resultValueVector.getOverflowBuffer());
    }
};

} // namespace operation
} // namespace function
} // namespace graphflow
