#include <gtest/gtest.h>
#include "../include/order_book.h"

class OrderBookTest : public ::testing::Test {
protected:
    const uint32_t MAX_TICKS = 100000; // $1000.00 max price at $0.01 increments
    const size_t MAX_ORDERS = 100000;  // Pre-allocate space for 100,000 active orders
};

TEST_F(OrderBookTest, Initialization) {
    LimitOrderBook book(MAX_TICKS, MAX_ORDERS);

    EXPECT_EQ(book.get_best_bid(), 0);
    EXPECT_EQ(book.get_best_ask(), MAX_TICKS);
}

TEST_F(OrderBookTest, AddSingleBid) {
    LimitOrderBook book(MAX_TICKS, MAX_ORDERS);
    
    uint64_t order_id = 1;
    uint32_t price_tick = 5000; // $50.00
    uint32_t qty = 100;
    bool is_buy = true;

    book.add_order(order_id, price_tick, qty, is_buy);

    // The best bid should now be updated to new order's price
    EXPECT_EQ(book.get_best_bid(), price_tick);
    
    // The best ask should remain unchanged at the default maximum
    EXPECT_EQ(book.get_best_ask(), MAX_TICKS);
}

TEST_F(OrderBookTest, AddSingleAsk) {
    LimitOrderBook book(MAX_TICKS, MAX_ORDERS);
    
    uint64_t order_id = 2;
    uint32_t price_tick = 5100; // $51.00
    uint32_t qty = 50;
    bool is_buy = false;

    book.add_order(order_id, price_tick, qty, is_buy);

    // The best ask should update to the new ask's price
    EXPECT_EQ(book.get_best_ask(), price_tick);
    
    // The best bid should remain unchanged at 0
    EXPECT_EQ(book.get_best_bid(), 0);
}

TEST_F(OrderBookTest, UpdateSpreadMultipleOrders) {
    LimitOrderBook book(MAX_TICKS, MAX_ORDERS);
    
    // Add multiple bids
    book.add_order(1, 4900, 100, true);
    book.add_order(2, 5000, 100, true); // Higher bid
    book.add_order(3, 4950, 100, true);

    // Add multiple asks
    book.add_order(4, 5200, 100, false);
    book.add_order(5, 5100, 100, false); // Lower ask
    book.add_order(6, 5150, 100, false);

    // The spread should be tightest around the highest bid and lowest ask
    EXPECT_EQ(book.get_best_bid(), 5000);
    EXPECT_EQ(book.get_best_ask(), 5100);
}

TEST_F(OrderBookTest, CancelOrder) {
    LimitOrderBook book(MAX_TICKS, MAX_ORDERS);
    
    // Add an order that establishes the best bid
    book.add_order(10, 5000, 100, true);
    EXPECT_EQ(book.get_best_bid(), 5000);

    // Cancel the order
    book.cancel_order(10);

    // If the order is correctly severed and the queue updates, the best bid should drop 
    // (Assuming cancel_order logic scans down to the next available bid, or returns to 0)
    EXPECT_EQ(book.get_best_bid(), 0); 
}

TEST_F(OrderBookTest, ExecuteMarketOrderFullFill) {
    LimitOrderBook book(MAX_TICKS, MAX_ORDERS);
    
    // Setup a resting Ask for 100 shares at $51.00
    book.add_order(20, 5100, 100, false);
    EXPECT_EQ(book.get_best_ask(), 5100);

    // Execute a Market Buy for exactly 100 shares
    book.execute_market_order(100, true);

    // The resting ask should be wiped out, resetting the best ask to MAX_TICKS
    EXPECT_EQ(book.get_best_ask(), MAX_TICKS);
}

TEST_F(OrderBookTest, ExecuteMarketOrderPartialFill) {
    LimitOrderBook book(MAX_TICKS, MAX_ORDERS);
    
    // Setup two resting Asks at the same price tick to test FIFO queue
    book.add_order(30, 5100, 100, false); // Front of queue
    book.add_order(31, 5100, 50, false);  // Back of queue

    // Execute a Market Buy for 120 shares
    // This should fully consume order 30, and take 20 shares from order 31
    book.execute_market_order(120, true);

    // The best ask should still be 5100 because order 31 has 30 shares remaining
    EXPECT_EQ(book.get_best_ask(), 5100);
}