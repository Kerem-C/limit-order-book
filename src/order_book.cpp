#include "../include/order_book.h"

void LimitOrderBook::add_order(uint64_t order_id, uint32_t price_tick, uint32_t quantity, bool is_buy) noexcept {
    if (price_tick >= max_price) [[unlikely]] return; 
    if (order_id >= max_orders) [[unlikely]] return;

    Order* new_order = memory_pool.allocate(order_id, price_tick, quantity, is_buy);

    order_map[order_id] = new_order;
    PriceLevel& level = price_levels[price_tick];

    // Maintain FIFO execution priority at the tail of the linked list
    if (level.tail == nullptr) {
        level.head = new_order;
        level.tail = new_order;
    } else {
        level.tail->next = new_order;
        new_order->prev = level.tail;
        level.tail = new_order;
    }

    level.total_volume += quantity;

    // Dynamically track the inside spread to prevent O(N) searches during market order execution
    if (is_buy) {
        if (price_tick > best_bid_tick || best_bid_tick == 0) {
            best_bid_tick = price_tick;
        }
    } else {
        if (price_tick < best_ask_tick || best_ask_tick == price_levels.size()) {
            best_ask_tick = price_tick;
        }
    }
}

void LimitOrderBook::cancel_order(uint64_t order_id) noexcept {
    if (order_id >= max_orders) [[unlikely]] return;

    Order* target = order_map[order_id];
    if (target == nullptr) [[unlikely]] return;

    PriceLevel& level = price_levels[target->price_tick];

    // Sever the O(1) Doubly-Linked List connections
    if (target->prev != nullptr) {
        target->prev->next = target->next;
    } else {
        // The canceled order was at the front of the line
        level.head = target->next;
    }

    if (target->next != nullptr) {
        target->next->prev = target->prev;
    } else {
        // The canceled order was at the back of the line
        level.tail = target->prev;
    }

    level.total_volume -= target->quantity;
    order_map[order_id] = nullptr;

    // O(1) Spread Recalculation using Bitboards
    if (level.head == nullptr) {
        if (target->is_buy) {
            bid_bitfield.clear_tick(max_price - target->price_tick);
            if (target-> price_tick == best_bid_tick) {
                int64_t next_bid = bid_bitfield.get_next_active_tick(max_price - best_bid_tick);
                best_bid_tick = (next_bid == -1) ? 0 : static_cast<uint32_t>(max_price - next_bid);
            }
        } else {
            ask_bitfield.clear_tick(target->price_tick);
            if (target->price_tick == best_ask_tick) {
                int64_t next_ask = ask_bitfield.get_next_active_tick(best_ask_tick);
                best_ask_tick = (next_ask == -1) ? max_price : static_cast<uint32_t>(next_ask);
            }
        }
    }

    memory_pool.deallocate(target);
}

void LimitOrderBook::execute_market_order(uint32_t quantity, bool is_buy) noexcept {
    uint32_t remaining_qty = quantity;

    while (remaining_qty > 0) {
        uint32_t current_best_tick = is_buy ? best_ask_tick : best_bid_tick;

        // Guard against infinite loops if the opposing book liquidity is entirely exhausted
        if (is_buy && current_best_tick == price_levels.size()) break;
        if (!is_buy && current_best_tick == 0 && price_levels[0].head == nullptr) break;

        PriceLevel& level = price_levels[current_best_tick];
        Order* resting_order = level.head;

        // Self-healing state correction if spread trackers point to a stale, empty level
        if (resting_order == nullptr) {
            if (is_buy) best_ask_tick++; else best_bid_tick--;
            continue;
        }

        if (resting_order->quantity <= remaining_qty) {
            remaining_qty -= resting_order->quantity;
            level.total_volume -= resting_order->quantity;
        
            level.head = resting_order->next;
            if (level.head != nullptr) {
                level.head->prev = nullptr;
            } else {
                level.tail = nullptr;
            }
            
            // Direct erasure avoids triggering the O(1) hash map lookup overhead of cancel_order()
            order_map[resting_order->order_id] = nullptr;
            memory_pool.deallocate(resting_order);

        } else {
            // Mutate the resting order in-place to preserve its queue time priority
            resting_order->quantity -= remaining_qty;
            level.total_volume -= remaining_qty;
            remaining_qty = 0;
        }

        // Hardware-Accelerated O(1) Scan to find the new inside market
        if (level.head == nullptr) {
            if (is_buy) {
                ask_bitfield.clear_tick(current_best_tick);
                int64_t next_ask = ask_bitfield.get_next_active_tick(current_best_tick);
                best_ask_tick = (next_ask == -1) ?  max_price : static_cast<uint32_t>(next_ask);
            } else {
                bid_bitfield.clear_tick(max_price - current_best_tick);
                int64_t next_bid = bid_bitfield.get_next_active_tick(max_price - current_best_tick);
                best_bid_tick = (next_bid == -1) ? 0 : static_cast<uint32_t>(max_price - next_bid);
            }
        }
    }
}