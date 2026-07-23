# High-Frequency Trading Limit Order Book

A deterministic, ultra-low latency limit order book (LOB) matching engine implemented in standard C++. 

## Purpose & Architecture
This project was engineered to explore market microstructure and push the limits of high-performance C++ system design. In quantitative finance, the difference between a profitable trade and a missed opportunity is measured in nanoseconds. Standard data structures inherently introduce unpredictable latency spikes due to heap allocations and linear scanning. 

To solve this, this engine bypasses standard library overhead by implementing hardware-accelerated, zero-allocation algorithms to achieve deterministic `O(1)` performance across all critical path operations.

### Key Optimizations
* **Zero-Allocation Memory Pool:** An upfront, statically-sized `OrderPool` handles all order creation and destruction. By popping and pushing memory addresses from a pre-allocated stack, the engine completely avoids the overhead of runtime `new`/`delete` calls.
* **Hardware-Accelerated Bitfield Indexing:** To avoid `O(N)` linear scans when traversing empty price levels, the engine tracks liquidity using a custom `BitfieldIndex`. By utilizing `std::countr_zero` (which compiles to single-cycle hardware instructions like `TZCNT` or `BSF`), the engine instantly jumps to the next active price tick, regardless of the spread width.
* **O(1) Order Cancellation:** A flat `std::unordered_map` stores memory pointers to active orders, allowing for instant hash-based retrieval. Doubly-linked lists at each price level allow orders to be severed and patched in constant time without disrupting FIFO execution priority.

## Performance Metrics
Performance is verified using Google Benchmark on a Release build (C++20, MSVC/GCC -O3). 

By transitioning from standard linear `while` loops to the 64-bit hardware-accelerated index, market order execution latency was reduced by approximately **100x**.

* **Limit Order Insertion:** ~197 ns
* **Market Order Execution & Spread Resolution:** ~474 ns

## Build & Test Instructions
The project utilizes CMake and integrates Google Test (GTest) for unit validation and Google Benchmark for microsecond-level performance tracking.

```bash
# Clone the repository
git clone [https://github.com/yourusername/limit-order-book.git](https://github.com/yourusername/limit-order-book.git)
cd limit-order-book

# Configure and build in Release mode
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run the test suites
./tests/order_book_tests
./tests/bitfield_tests

# Run the performance benchmarks
./benchmarks/bench_order_book