cmake_minimum_required(VERSION 3.10)
project(LimitOrderBookEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall -Wextra")
endif()
include_directories(include)

add_executable(matching_engine src/main.cpp src/order_book.cpp)

# ======================
# Testing (Google Test)
# ======================
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(order_book_tests tests/test_order_book.cpp)
add_executable(bitfield_tests tests/test_bitfield_index.cpp)
target_sources(order_book_tests PRIVATE src/order_book.cpp)
target_link_libraries(order_book_tests gtest_main)
target_link_libraries(bitfield_tests gtest_main)

include(GoogleTest)
gtest_discover_tests(order_book_tests)
gtest_discover_tests(bitfield_tests)

# ======================
# Benchmarking (Google Benchmark)
# ======================
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3 
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE) 
FetchContent_MakeAvailable(benchmark)

add_executable(bench_order_book benchmarks/benchmark_order_book.cpp)
target_sources(bench_order_book PRIVATE src/order_book.cpp)
target_link_libraries(bench_order_book PRIVATE benchmark::benchmark benchmark::benchmark_main)

# ======================
# Networking (Standalone ASIO)
# ======================
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-30-2 
)
FetchContent_MakeAvailable(asio)

add_executable(lob_server src/server_main.cpp src/order_book.cpp)
target_include_directories(lob_server PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(lob_server PRIVATE ASIO_STANDALONE)

if(WIN32)
    target_link_libraries(lob_server PRIVATE wsock32 ws2_32)
endif()