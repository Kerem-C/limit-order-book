#include <benchmark/benchmark.h>
#include "order_book.h"

static void BM_AddSingleOrder(benchmark::State& state) {
    LimitOrderBook lob(100000, 100000); 

    for (auto _ : state) {
        lob.add_order(1, 15000, 100, false);
        state.PauseTiming();
        lob.cancel_order(1);
        state.ResumeTiming();
        benchmark::DoNotOptimize(lob); 
    }
}
BENCHMARK(BM_AddSingleOrder);

static void BM_ExecuteMarketOrder(benchmark::State& state) {
    LimitOrderBook lob(100000, 100000);

    for (auto _ : state) {
        state.PauseTiming();
        lob.add_order(2, 15000, 100, false);
        state.ResumeTiming();

        // This triggers the while-loop linear scan to find the next best ask
        lob.execute_market_order(100, true);
    }
}
BENCHMARK(BM_ExecuteMarketOrder);

BENCHMARK_MAIN();