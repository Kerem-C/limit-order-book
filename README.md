# High-Frequency Trading Limit Order Book

A low-latency, $O(1)$ limit order book matching engine implemented in C++. 

## Overview
This core execution engine is designed for ultra-low latency environments. It utilizes constant-time data structures to manage liquidity, process incoming market/limit orders, and maintain strict price-time priority. 

## Current Performance
- **Language:** Standard C++ 
- **Build System:** CMake / MSVC
- **Execution Time:** ~92.8 microseconds (Release build) for standard market order matching.

## Architecture
- **`src/order_book.cpp`**: Contains the core matching logic (`add_order`, `cancel_order`, `execute_market_order`).
- **`include/order_book.h`**: Data structure definitions and class declarations.

## Next Steps
- Implementing zero-allocation custom memory pools.
- Integrating Google Test (GTest) for unit validation.
- Integrating Google Benchmark for microsecond-level performance tracking.
- Network data ingestion (TCP/UDP).