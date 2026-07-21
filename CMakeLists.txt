cmake_minimum_required(VERSION 3.10)
project(LimitOrderBookEngine)

set(CMAKE_CXX_STANDARD 17)
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
target_sources(order_book_tests PRIVATE src/order_book.cpp)
target_link_libraries(order_book_tests gtest_main)

include(GoogleTest)
gtest_discover_tests(order_book_tests)

# ======================
# Benchmarking (Google Benchmark)
# ======================
include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3 
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE) 
FetchContent_MakeAvailable(benchmark)
add_executable(bench_order_book benchmarks/benchmark_order_book.cpp)
target_link_libraries(bench_order_book PRIVATE benchmark::benchmark benchmark::benchmark_main)