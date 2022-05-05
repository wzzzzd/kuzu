#pragma once

#include "normalized_single_query.h"

namespace graphflow {
namespace binder {

class BoundRegularQuery {

public:
    explicit BoundRegularQuery(vector<bool> isUnionAll) : isUnionAll{move(isUnionAll)} {}

    ~BoundRegularQuery() = default;

    inline void addSingleQuery(unique_ptr<NormalizedSingleQuery> singleQuery) {
        singleQueries.push_back(move(singleQuery));
    }

    inline uint64_t getNumSingleQueries() const { return singleQueries.size(); }

    inline NormalizedSingleQuery* getSingleQuery(uint32_t idx) const {
        return singleQueries[idx].get();
    }

    inline bool getIsUnionAll(uint32_t idx) const { return isUnionAll[idx]; }

    // For regular query (i.e. union of single queries), since there
    inline expression_vector getExpressionsToReturn() const {
        assert(!singleQueries.empty());
        return singleQueries[0]->getExpressionsToReturn();
    }

private:
    vector<unique_ptr<NormalizedSingleQuery>> singleQueries;
    vector<bool> isUnionAll;
};

} // namespace binder
} // namespace graphflow
