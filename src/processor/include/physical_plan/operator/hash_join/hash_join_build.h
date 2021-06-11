#pragma once

#include "src/common/include/memory_manager.h"
#include "src/common/include/operations/hash_operations.h"
#include "src/common/include/types.h"
#include "src/common/include/vector/operations/vector_node_id_operations.h"
#include "src/processor/include/physical_plan/operator/physical_operator.h"
#include "src/processor/include/physical_plan/operator/result/result_set.h"
#include "src/processor/include/physical_plan/operator/sink/sink.h"

using namespace std;
using namespace graphflow::common;
using namespace graphflow::common::operation;

namespace graphflow {
namespace processor {

// By default, ht block size is 256KB
constexpr const uint64_t DEFAULT_HT_BLOCK_SIZE = 1 << 18;
// By default, overflow block size is 2MB
constexpr const int64_t DEFAULT_OVERFLOW_BLOCK_SIZE = 1 << 21;

struct BlockAppendInfo {
    BlockAppendInfo(uint8_t* buffer, uint64_t numEntries)
        : buffer(buffer), numEntries(numEntries) {}

    uint8_t* buffer;
    uint64_t numEntries;
};

struct BlockHandle {
public:
    explicit BlockHandle(unique_ptr<MemoryBlock> block, uint64_t numEntries)
        : data{block->data}, freeSize{block->size}, numEntries{numEntries}, block{move(block)} {}

public:
    uint8_t* data;
    uint64_t freeSize;
    uint64_t numEntries;

private:
    unique_ptr<MemoryBlock> block;
};

// This is a shared state between HashJoinBuild and HashJoinProbe operators.
// Each clone of these two operators will share the same state.
// Inside the state, we keep the number of tuples inside the hash table (numEntries) a global
// list of references to ht blocks (htBlocks) and also overflow blocks (overflowBlocks), which
// are merged by each HashJoinBuild thread when they finished materializing thread-local tuples.
// Also, the state holds a global htDirectory, which will be updated by the last thread in the
// hash join build side task/pipeline, and probed by the HashJoinProbe operators.
class HashJoinSharedState {
public:
    HashJoinSharedState(uint64_t numBytesForFixedTuplePart)
        : htDirectory(nullptr), numBytesForFixedTuplePart(numBytesForFixedTuplePart), numEntries(0),
          hashBitMask(0){};

    mutex hashJoinSharedStateLock;

    unique_ptr<MemoryBlock> htDirectory;
    vector<BlockHandle> htBlocks;
    vector<BlockHandle> overflowBlocks;
    uint64_t numBytesForFixedTuplePart;
    uint64_t numEntries;
    uint64_t hashBitMask;
};

class HashJoinBuild : public Sink {
public:
    HashJoinBuild(uint64_t keyDataChunkPos, uint64_t keyVectorPos,
        vector<bool> dataChunkPosToIsFlat, unique_ptr<PhysicalOperator> prevOperator,
        ExecutionContext& context, uint32_t id);

    void getNextTuples() override;

    unique_ptr<PhysicalOperator> clone() override {
        auto cloneOp = make_unique<HashJoinBuild>(keyDataChunkPos, keyVectorPos,
            dataChunkPosToIsFlat, prevOperator->clone(), context, id);
        cloneOp->sharedState = this->sharedState;
        cloneOp->memManager = this->memManager;
        return cloneOp;
    }

    // Finalize the hash table directory
    void finalize() override;

    // The memory manager should be set when processor prepares pipeline tasks
    inline void setMemoryManager(MemoryManager* memManager) { this->memManager = memManager; }

    shared_ptr<HashJoinSharedState> sharedState;
    uint64_t numBytesForFixedTuplePart;

private:
    MemoryManager* memManager;
    uint64_t keyDataChunkPos;
    uint64_t keyVectorPos;
    vector<bool> dataChunkPosToIsFlat;

    shared_ptr<DataChunk> keyDataChunk;
    shared_ptr<ResultSet> resultSetWithoutKeyDataChunk;

    uint64_t htBlockCapacity;
    uint64_t numEntries; // Thread-local num entries in htBlocks

    // Thread local main memory blocks holding |key|payload|prev| fields
    vector<BlockHandle> htBlocks;
    // Thread local overflow memory blocks for variable-sized values in tuples
    vector<BlockHandle> overflowBlocks;

    void allocateHTBlocks(uint64_t remaining, vector<BlockAppendInfo>& tuplesAppendInfos);

    void appendPayloadVectorAsFixSizedValues(ValueVector& vector, uint8_t* appendBuffer,
        uint64_t valueOffsetInVector, uint64_t appendCount, bool isSingleValue);
    void appendPayloadVectorAsOverflowValue(
        ValueVector& vector, uint8_t* appendBuffer, uint64_t appendCount);
    void appendKeyVector(NodeIDVector& vector, uint8_t* appendBuffer, uint64_t valueOffsetInVector,
        uint64_t appendCount) const;
    overflow_value_t addVectorInOverflowBlocks(ValueVector& vector);

    void appendResultSet();
};
} // namespace processor
} // namespace graphflow
