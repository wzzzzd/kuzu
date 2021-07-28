#include <iostream>

#include "gtest/gtest.h"

#include "src/processor/include/physical_plan/operator/read_list/frontier/frontier_set.h"

using namespace graphflow::processor;

TEST(FrontierTests, frontierCreationTest) {
    MemoryManager memMan;

    NodeIDCompressionScheme compressionScheme;
    NodeIDVector vector = NodeIDVector(0, compressionScheme, false);
    vector.state = make_shared<DataChunkState>(DEFAULT_VECTOR_CAPACITY);
    vector.state->size = DEFAULT_VECTOR_CAPACITY;
    for (auto i = 0u; i < vector.state->size; i++) {
        ((node_offset_t*)vector.values)[i] = i;
    }

    auto FIXED_MULTIPLICITY_VALUE = 12u;
    auto frontierBag = FrontierBag();
    frontierBag.setMemoryManager(&memMan);
    frontierBag.initHashTable();
    frontierBag.append(vector, FIXED_MULTIPLICITY_VALUE);
    for (auto i = 0u; i < 10; i++) {
        for (auto j = 0u; j < vector.state->size; j++) {
            ((node_offset_t*)vector.values)[j] =
                ((node_offset_t*)vector.values)[j] + DEFAULT_VECTOR_CAPACITY;
        }
        frontierBag.append(vector, FIXED_MULTIPLICITY_VALUE);
    }
    ASSERT_EQ(frontierBag.size, DEFAULT_VECTOR_CAPACITY * 11);

    auto highestID = DEFAULT_VECTOR_CAPACITY * 11;
    for (auto i = 0u; i < NUM_SLOTS_BAG; i++) {
        auto slot = frontierBag.hashTable[i];
        auto arrSize =
            slot.size < NODE_IDS_PER_SLOT_FOR_BAG ? slot.size : NODE_IDS_PER_SLOT_FOR_BAG;
        node_offset_t nodeOffset;
        for (auto j = 0u; j < arrSize; j++) {
            nodeOffset = j * NUM_SLOTS_BAG + i;
            ASSERT_EQ(slot.nodeOffsets[j], nodeOffset % highestID);
            ASSERT_EQ(slot.multiplicity[j], FIXED_MULTIPLICITY_VALUE);
        }
        if (slot.size > NODE_IDS_PER_SLOT_FOR_BAG) {
            auto value = slot.next;
            for (auto j = NUM_SLOTS_BAG + nodeOffset; j < vector.state->size; j += NUM_SLOTS_BAG) {
                ASSERT_EQ(value->nodeOffset, j % highestID);
                ASSERT_EQ(value->multiplicity, FIXED_MULTIPLICITY_VALUE);
                value = value->next;
            }
        }
    }

    auto frontier = FrontierSet();
    frontier.setMemoryManager(&memMan);
    auto numSlots = 64;
    frontier.initHashTable(numSlots);
    for (auto slot = 0; slot < NUM_SLOTS_BAG; slot++) {
        frontier.insert(frontierBag, slot);
    }
    for (auto i = 0u; i < numSlots; i++) {
        auto slot = &frontier.hashTableBlocks[0][i];
        auto offset = i;
        if (slot->hasValue) {
            ASSERT_EQ(slot->nodeOffset, offset);
            ASSERT_EQ(slot->multiplicity, FIXED_MULTIPLICITY_VALUE);
            while (slot->next) {
                offset += numSlots;
                slot = slot->next;
                ASSERT_EQ(slot->nodeOffset, offset);
                ASSERT_EQ(slot->multiplicity, FIXED_MULTIPLICITY_VALUE);
            }
        }
    }
}
