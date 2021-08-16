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

        // Upcasting
        GrandParent* pg;
        benchmark::DoNotOptimize(pg = dynamic_cast<GrandParent*>(&c));

        ParentA* pa;
        benchmark::DoNotOptimize(pa = dynamic_cast<ParentA*>(&c));

        ParentB* pb;
        benchmark::DoNotOptimize(pb = dynamic_cast<ParentB*>(&c));

        InvalidCast* invalid;
        benchmark::DoNotOptimize(invalid = dynamic_cast<InvalidCast*>(&c));

        // Downcasting
        Child* pc;
        benchmark::DoNotOptimize(pc = dynamic_cast<Child*>(pg));
        benchmark::DoNotOptimize(pc = dynamic_cast<Child*>(pa));
        benchmark::DoNotOptimize(pc = dynamic_cast<Child*>(pb));
    }
}
BENCHMARK(NativeDynamicCast);

static void
RttiDynamicCast(benchmark::State& state) {
    for (auto _ : state) {
        Child c;

        // Upcasting
        GrandParent* pg;
        benchmark::DoNotOptimize(pg = c.cast<GrandParent>());

        ParentA* pa;
        benchmark::DoNotOptimize(pa = c.cast<ParentA>());

        ParentB* pb;
        benchmark::DoNotOptimize(pb = c.cast<ParentB>());

        InvalidCast* invalid;
        benchmark::DoNotOptimize(invalid = c.cast<InvalidCast>());

        // Downcasting
        Child* pc;
        benchmark::DoNotOptimize(pc = pg->cast<Child>());
        benchmark::DoNotOptimize(pc = pa->cast<Child>());
        benchmark::DoNotOptimize(pc = pb->cast<Child>());
    }
}
BENCHMARK(RttiDynamicCast);

BENCHMARK_MAIN();