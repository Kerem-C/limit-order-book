#include <iostream>
#include <chrono>
#include "../include/order_book.h"

int main() {
    // Initialize the engine with a max price of $1,000.00 (100,000 ticks of $0.01)
    LimitOrderBook engine(100000);

    std::cout << "--- Exchange Booted ---" << std::endl;

    // 1. Build the Order Book (Injecting Limit Orders)
    // format: order_id, price_tick, quantity, is_buy
    engine.add_order(1, 15000, 100, false); // Sell 100 @ $150.00
    engine.add_order(2, 15005, 200, false); // Sell 200 @ $150.05
    engine.add_order(3, 14995, 50, true);   // Buy 50 @ $149.95
    engine.add_order(4, 14990, 300, true);  // Buy 300 @ $149.90

    std::cout << "Liquidity injected. Best Ask: " << engine.get_best_ask() 
              << " | Best Bid: " << engine.get_best_bid() << std::endl;

    // 2. Execute a Cancellation
    engine.cancel_order(2); // Instantly cancel the $150.05 sell order
    std::cout << "Order #2 Canceled." << std::endl;

    // 3. Execute a Market Order
    // A trader wants to buy 150 shares immediately.
    // This should consume all 100 shares at $150.00, and then step up to the next available ask.
    std::cout << "Executing Market Buy for 150 shares..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    engine.execute_market_order(150, true);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "Market order matched in " << duration.count() << " nanoseconds." << std::endl;
    std::cout << "New Best Ask: " << engine.get_best_ask() << std::endl;

    return 0;
}