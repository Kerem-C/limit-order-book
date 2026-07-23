#include <gtest/gtest.h>
#include "../include/bitfield_index.h"

class BitfieldIndexTest : public ::testing::Test {
protected:
    // 100,000 ticks covers $1,000.00 at $0.01 increments
    const size_t MAX_TICKS = 100000;
};

TEST_F(BitfieldIndexTest, BasicSetAndClear) {
    BitfieldIndex index(MAX_TICKS);

    // Book initially empty
    EXPECT_EQ(index.get_next_active_tick(0), -1);

    index.set_tick(5);

    // Scanning from 0 or exactly 5 should find tick 5 immediately
    EXPECT_EQ(index.get_next_active_tick(0), 5);
    EXPECT_EQ(index.get_next_active_tick(5), 5);

    EXPECT_EQ(index.get_next_active_tick(6), -1);

    index.clear_tick(5);
    EXPECT_EQ(index.get_next_active_tick(0), -1);
}

TEST_F(BitfieldIndexTest, MultipleBlocksAndMasking) {
    BitfieldIndex index(MAX_TICKS);

    index.set_tick(70); // Block 1 (starts from 0), Bit 6
    index.set_tick(15000); // Block 234

    EXPECT_EQ(index.get_next_active_tick(0), 70);
    EXPECT_EQ(index.get_next_active_tick(71), 15000);
}