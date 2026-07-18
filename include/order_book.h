#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

// 1. The Atomic Unit: A single limit order
// Implemented as a Doubly-Linked List to enable O(1) memory severing
struct Order {
    uint64_t order_id;
    uint32_t price_tick; // Price normalized to an integer index (e.g., $100.00 --> 10000)
    uint32_t quantity;
    bool is_buy;

    Order* prev;
    Order* next;

    Order(uint64_t id, uint32_t p, uint32_t q, bool buy)
        : order_id(id), price_tick(p), quantity(q), is_buy(buy), prev(nullptr), next(nullptr) {}
};

// 2. The Queue: All the orders sitting at a specific price tick
// Maintains FIFO Priority
struct PriceLevel {
    Order* head;
    Order* tail;
    uint64_t total_volume;

    PriceLevel() : head(nullptr), tail(nullptr), total_volume(0) {}
};

// 3. The Engine: The Limit Order Book Memory Layout
class LimitOrderBook {
private:
    // O(1) Price-Indexed Array
    // If max price is $1,000.00 with $0.01 ticks, this holds 100,000 PriceLevels
    std::vector<PriceLevel> price_levels;

    // O(1) Cancellation Map
    // Maps unique order_id to memory address in the system
    std::unordered_map<uint64_t, Order*> order_map;

    uint32_t best_bid_tick;
    uint32_t best_ask_tick;

public:
    // Allocate entire array upfront to prevent runtime memory reallocation
    LimitOrderBook(uint32_t max_price_ticks) {
        price_levels.resize(max_price_ticks);
        best_bid_tick = 0;
        best_ask_tick = max_price_ticks;
    }

    inline uint32_t get_best_bid() const { return best_bid_tick; }
    inline uint32_t get_best_ask() const { return best_ask_tick; }

    void add_order(uint64_t order_id, uint32_t price_tick, uint32_t quantity, bool is_buy);
    void cancel_order(uint64_t order_id);
    void execute_market_order(uint32_t quantity, bool is_buy);
};