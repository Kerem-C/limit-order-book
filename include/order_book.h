#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <stdexcept>
#include "bitfield_index.h"

// The Atomic Unit
struct Order {
    uint64_t order_id;
    uint32_t price_tick; // Price normalized to an integer index (e.g., $100.00 --> 10000)
    uint32_t quantity;
    bool is_buy;

    Order* prev;
    Order* next;

    Order() : order_id(0), price_tick(0), quantity(0), is_buy(false), prev(nullptr), next(nullptr) {}
};

// Custom Memory Allocator
class OrderPool {
private:
    std::vector<Order> pool;
    std::vector<Order*> free_list;

public:
    // Allocate entire memory block ONCE at startup
    OrderPool(size_t max_active_orders) {
        pool.resize(max_active_orders);
        free_list.reserve(max_active_orders);

        // Push the addresses of all pre-allocated orders into the free list stack
        for (size_t i = 0; i < max_active_orders; ++i) {
            free_list.push_back(&pool[i]);
        }
    }

    // O(1) Allocation: Pop a pre-existing memory address off the stack
    [[nodiscard]] inline Order* allocate(uint64_t id, uint32_t p, uint32_t q, bool buy) {
        if (free_list.empty()) {
            throw std::runtime_error("OrderPool exhausted: Max capacity reached.");
        }
        Order* order = free_list.back();
        free_list.pop_back();

        order->order_id = id;
        order->price_tick = p;
        order->quantity = q;
        order->is_buy = buy;
        order->prev = nullptr;
        order->next = nullptr;

        return order;
    }

    // O(1) Deallocation: Push the memory address back onto the stack
    inline void deallocate(Order* order) {
        free_list.push_back(order);
    }
};

// The Queue
struct PriceLevel {
    Order* head;
    Order* tail;
    uint64_t total_volume;

    PriceLevel() : head(nullptr), tail(nullptr), total_volume(0) {}
};

// The Engine
class LimitOrderBook {
private:
    // O(1) Price-Indexed Array
    // If max price is $1,000.00 with $0.01 ticks, this holds 100,000 PriceLevels
    std::vector<PriceLevel> price_levels;

    // O(1) Cancellation Map
    std::vector<Order*> order_map;

    OrderPool memory_pool;

    BitfieldIndex bid_bitfield;
    BitfieldIndex ask_bitfield;
    uint32_t max_price;
    size_t max_orders;

    uint32_t best_bid_tick;
    uint32_t best_ask_tick;

public:
    // Allocate entire array upfront to prevent runtime memory reallocation
    LimitOrderBook(uint32_t max_price_ticks, size_t max_active_orders)
        : memory_pool(max_active_orders),
          bid_bitfield(max_price_ticks + 1),
          ask_bitfield(max_price_ticks + 1),
          max_price(max_price_ticks),
          max_orders(max_active_orders) {

        price_levels.resize(max_price_ticks);
        order_map.resize(max_active_orders, nullptr);

        best_bid_tick = 0;
        best_ask_tick = max_price_ticks;
    }

    [[nodiscard]] inline uint32_t get_best_bid() const { return best_bid_tick; }
    [[nodiscard]] inline uint32_t get_best_ask() const { return best_ask_tick; }

    void add_order(uint64_t order_id, uint32_t price_tick, uint32_t quantity, bool is_buy) noexcept;
    void cancel_order(uint64_t order_id) noexcept;
    void execute_market_order(uint32_t quantity, bool is_buy) noexcept;
};