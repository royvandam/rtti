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

struct Child
    : ParentA
    , ParentB {
    RTTI_DECLARE_TYPEINFO(Child, ParentA, ParentB);
};

struct InvalidCast {};

static void
NativeDynamicCast(benchmark::State& state) {
    for (auto _ : state) {
        Child c;

        GrandParent* g;
        benchmark::DoNotOptimize(g = dynamic_cast<GrandParent*>(&c));

        ParentA* pa;
        benchmark::DoNotOptimize(pa = dynamic_cast<ParentA*>(g));

        ParentB* pb;
        benchmark::DoNotOptimize(pb = dynamic_cast<ParentB*>(g));

        InvalidCast* invalid;
        benchmark::DoNotOptimize(invalid = dynamic_cast<InvalidCast*>(g));
    }
}
BENCHMARK(NativeDynamicCast);

static void
RttiDynamicCast(benchmark::State& state) {
    for (auto _ : state) {
        Child c;

        GrandParent* g;
        benchmark::DoNotOptimize(g = c.cast<GrandParent>());

        ParentA* pa;
        benchmark::DoNotOptimize(pa = g->cast<ParentA>());

        ParentB* pb;
        benchmark::DoNotOptimize(pb = g->cast<ParentB>());

        InvalidCast* invalid;
        benchmark::DoNotOptimize(invalid = g->cast<InvalidCast>());
    }
}
BENCHMARK(RttiDynamicCast);

BENCHMARK_MAIN();