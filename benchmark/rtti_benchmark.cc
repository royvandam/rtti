#include <benchmark/benchmark.h>
#include <rtti.hh>

struct GrandParentA : RTTI::Base<GrandParentA> {};
struct ParentA : RTTI::Extends<ParentA, GrandParentA> {};
struct GrandParentB : RTTI::Base<GrandParentB> {};
struct ParentB : RTTI::Extends<ParentB, GrandParentB> {};
struct Class : RTTI::Extends<Class, ParentA, ParentB> {
    using GrandParentA::cast;
};
struct InvalidCast {};

static void NativeDynamicCast(benchmark::State& state) {
    Class c;
    GrandParentA* gpa; GrandParentB* gpb;
    InvalidCast *invalid;

    for (auto _ : state) {
        benchmark::DoNotOptimize(gpa = dynamic_cast<GrandParentA*>(&c));
        benchmark::DoNotOptimize(gpb = dynamic_cast<GrandParentB*>(gpa));
        benchmark::DoNotOptimize(invalid = dynamic_cast<InvalidCast*>(gpa));
    }
}
// Register the function as a benchmark
BENCHMARK(NativeDynamicCast);

// Define another benchmark
static void RttiDynamicCast(benchmark::State& state) {
    Class c;
    GrandParentA* gpa; GrandParentB* gpb;
    InvalidCast *invalid;

    for (auto _ : state) {
        benchmark::DoNotOptimize(gpa = c.cast<GrandParentA*>());
        benchmark::DoNotOptimize(gpb = gpa->cast<GrandParentB*>());
        benchmark::DoNotOptimize(invalid = c.cast<InvalidCast*>());
    }
}
BENCHMARK(RttiDynamicCast);

BENCHMARK_MAIN();