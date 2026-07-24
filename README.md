# C++ High-Frequency Limit Order Book (LOB)

A deterministic, ultra-low latency limit order book matching engine implemented in standard C++20. 

## Architecture & Design Philosophy
This project was engineered to explore market microstructure and push the limits of high-performance C++ system design. Standard data structures inherently introduce unpredictable latency spikes due to heap allocations and linear scanning. This engine bypasses standard library overhead by implementing hardware-accelerated, zero-allocation algorithms to achieve deterministic `O(1)` performance across critical path operations.

* **Zero-Allocation Memory Pool:** An upfront, statically-sized `OrderPool` handles all order creation and destruction. By utilizing a pre-allocated stack of pointers, the engine completely avoids the overhead of runtime `new`/`delete` calls.
* **Hardware-Accelerated Bitfield Indexing:** To avoid `O(N)` linear scans when traversing empty price levels, liquidity is tracked using a custom `BitfieldIndex`. Utilizing `std::countr_zero` (compiling to single-cycle `TZCNT`/`BSF` instructions), the engine instantly jumps to the next active price tick regardless of spread width.
* **O(1) Order Cancellation:** A pre-allocated hash map stores memory pointers to active orders, allowing for instant retrieval. Doubly-linked lists at each price level allow orders to be severed and patched in constant time while preserving FIFO execution priority.
* **Zero-Copy Network Ingestion:** Utilizes Standalone ASIO for asynchronous TCP ingestion. The binary protocol (`#pragma pack(push, 1)`) is read directly off the wire into memory-aligned structs, minimizing serialization overhead.

## Performance Metrics
Metrics were captured using Google Benchmark and custom load-testing scripts on a Release build (GCC/MSVC -O3, `-march=native`).

### Engine Micro-Benchmarks (Latency)
By transitioning from standard linear while-loops to the 64-bit hardware-accelerated bitfield index, market order execution latency was reduced by approximately **100x**.
* **Limit Order Insertion:** ~197 ns
* **Market Order Execution (with Spread Resolution):** ~474 ns

### Network & Throughput (TCP)
A Python-based load tester was used to blast a pre-compiled 17.17 MB binary payload of 1,000,000 orders over a local TCP socket.
* **Throughput:** 1,539,730 messages per second (MPS)
* **Average Network-to-Engine Processing:** ~649 ns per order
* **Client-Side Push (OS buffer):** ~3.0 ms

## Build Instructions
The project utilizes CMake and integrates Google Test for unit validation and Google Benchmark for microsecond-level performance tracking.

```bash
# Clone the repository
git clone [https://github.com/yourusername/limit-order-book.git](https://github.com/yourusername/limit-order-book.git)
cd limit-order-book

# Configure and build in Release mode
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run Validation & Benchmarks
./tests/order_book_tests
./tests/bitfield_tests
./benchmarks/bench_order_book