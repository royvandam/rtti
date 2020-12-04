# Hand rolled open-hierarchy RTTI for C++17 and up.

If you have ever attempted to use the C++'s build in RTTI on a resource constrained (embedded) system you will most likely have noticed it is massively inefficient. Hence this implementation of a hand-rolled form of RTTI which is much more efficient and flexible, although it requires a bit more work from you as a class author. It is inspired by the guidelines defined for RTTI by the LLVM project<sup>[1]</sup> but with the extension that it supports:

 - Automatic ID generation
 - Multiple inheritance (no diamond ¯\\_(ツ)_/¯)
 - Dynamic casting (supports pointer offsets<sup>[2]</sup>)

[1] https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html

[2] It works for the defined test cases, but needs to be proven at large.

---

## How to use

Add `-fno-rtti` to your compile options in order to disable C++'s build in RTTI.

The RTTI::Base class describes provides the implementation for performing RTTI checks and casts on the highest member of the class hierarchy. The RTTI::Extends class template provides an implementation for classes derived from RTTI::Base. Both the RTTI::Base and RTTI::Extends use the “Curiously Recurring Template Idiom”, taking the class being defined as its first template argument and the parent class as the second argument. Note that RTTI::Extends support multiple inheritance by allowing the pass a variadic number of parent classes. All parent classes passed need to be derived from either a RTTI::Base or RTTI::Extends in this case. It is possible to mixin types without RTTI, but this needs to be done outside of the template arguments.

Basic usage example:

```c++
struct Shape : RTTI::Base<Shape> {};
struct Square : RTTI::Extends<Square, Shape> {};
struct Circle : RTTI::Extends<Circle, Shape> {};

struct SpecialBase : RTTI::Base<SpecialBase> {};
struct SpecialCircle : RTTI::Extends<SpecialCircle, Circle, SpecialBase> {};

int main() {
    SpecialCircle c;
    Shape* shape = &c;

    if (shape->isA(TypeInfo<Circle>::Id()) {
        std::cout << "Yep, the shape is a circle!" << std::endl;
    }

    if (auto circle = shape->cast<Circle*>()) {
        std::cout << "Woot, we have a circle! \0/" << std::endl;
    }

    if (shape->cast<Square*>() == nullptr) {
        std::cout << "No, it is not a square... :(" << std::endl;        
    }

    if (auto base = shape->cast<SpecialBase*>()) {
        std::cout << "The shape can also be cast to the special base class." << std::endl;
    }

    return 0;
}

```

---

## Benchmark Results

```
roy@desktop:~/Projects/rtti$ ./build/benchmark/rtti-benchmark 
2020-12-04T09:47:46+01:00
Running ./build/benchmark/rtti-benchmark
Run on (6 X 4300 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 9216 KiB (x1)
Load Average: 0.67, 0.81, 0.98
------------------------------------------------------------
Benchmark                  Time             CPU   Iterations
------------------------------------------------------------
NativeDynamicCast        123 ns          123 ns      5585135
RttiDynamicCast         1.92 ns         1.92 ns    364474077
```