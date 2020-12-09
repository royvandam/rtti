#include <benchmark/benchmark.h>

#include <rtti.hh>

struct GrandParent : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(GrandParent);
};

struct ParentA : virtual GrandParent {
    RTTI_DECLARE_TYPEINFO(ParentA, GrandParent);
};

struct ParentB : virtual GrandParent {
    RTTI_DECLARE_TYPEINFO(ParentB, GrandParent);
};

struct Child : ParentA, ParentB {
    RTTI_DECLARE_TYPEINFO(Child, ParentA, ParentB);
};

struct InvalidCast {};

static void
NativeDynamicCast(benchmark::State& state) {
    Child c;
    GrandParent* g;
    ParentA *pa;
    ParentB *pb;
    InvalidCast* invalid;

    for (auto _ : state) {
        benchmark::DoNotOptimize(g = dynamic_cast<GrandParent*>(&c));
        benchmark::DoNotOptimize(pa = dynamic_cast<ParentA*>(g));
        benchmark::DoNotOptimize(pb = dynamic_cast<ParentB*>(g));
        benchmark::DoNotOptimize(invalid = dynamic_cast<InvalidCast*>(g));
    }
}
BENCHMARK(NativeDynamicCast);

static void
RttiDynamicCast(benchmark::State& state) {
    Child c;
    GrandParent* g;
    ParentA *pa;
    ParentB *pb;
    InvalidCast* invalid;

    for (auto _ : state) {
        benchmark::DoNotOptimize(g = c.cast<GrandParent>());
        benchmark::DoNotOptimize(pa = g->cast<ParentA>());
        benchmark::DoNotOptimize(pb = g->cast<ParentB>());
        benchmark::DoNotOptimize(invalid = g->cast<InvalidCast>());
    }
}
BENCHMARK(RttiDynamicCast);

BENCHMARK_MAIN();