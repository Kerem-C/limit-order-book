#pragma once
#include <vector>
#include <cstdint>
#include <bit>

class BitfieldIndex {
private:
    std::vector<uint64_t> blocks;
    size_t max_ticks;

public:
    explicit BitfieldIndex(size_t max_ticks) : max_ticks(max_ticks) {
        blocks.resize((max_ticks / 64) + 1, 0);
    }

    // Level has liquidity
    void set_tick(size_t tick) {
        size_t block_idx = tick / 64;
        size_t bit_idx = tick % 64;
        blocks[block_idx] |= (1ULL << bit_idx);
    }

    // Level is completely empty
    void clear_tick(size_t tick) {
        size_t block_idx = tick / 64;
        size_t bit_idx = tick % 64;
        blocks[block_idx] &= ~(1ULL << bit_idx);
    }

    int64_t get_next_active_tick(size_t starting_tick) const {
        size_t block_idx = starting_tick / 64;
        size_t bit_idx = starting_tick % 64;

        if (block_idx >= blocks.size()) return -1;

        // Mask out the bits *below* starting tick in the current block
        // to avoid accidentally matching a lower price.
        uint64_t current_block = blocks[block_idx];
        uint64_t mask = ~0ULL << bit_idx;
        current_block &= mask;

        if (current_block != 0) {
            return (block_idx * 64) + std::countr_zero(current_block);
        }

        for (size_t i = block_idx + 1; i < blocks.size(); ++i) {
            if (blocks[i] != 0) {
                return (i * 64) + std::countr_zero(blocks[i]);
            }
        }
        
        return -1;
    }
};