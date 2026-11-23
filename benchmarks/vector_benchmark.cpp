#include <benchmark/benchmark.h>
#include <vector>
#include "fast_ds/custom_vector.hpp"

static void BM_StdVectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> vec;
        for (int i = 0; i < state.range(0); ++i) {
            vec.push_back(i);
        }
        benchmark::DoNotOptimize(vec.data());
    }
}
BENCHMARK(BM_StdVectorPushBack)->Range(1024, 1 << 20);

static void BM_CustomVectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        fast_ds::CustomVector<int> vec;
        for (int i = 0; i < state.range(0); ++i) {
            vec.push_back(i);
        }
        benchmark::DoNotOptimize(vec);
    }
}
BENCHMARK(BM_CustomVectorPushBack)->Range(1024, 1 << 20);
