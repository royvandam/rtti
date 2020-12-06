# Hand rolled open-hierarchy RTTI for C++17 and up.

If you have ever attempted to use the C++'s build in RTTI on a resource constrained (embedded) system you will most likely have noticed it is massively inefficient. Hence this implementation of a hand-rolled form of RTTI which is much more efficient and flexible, although it requires a bit more work from you as a class author. It is inspired by the guidelines defined for RTTI by the LLVM project<sup>[1]</sup> but with the extension that it supports:

 - Automatic ID generation
 - Multiple inheritance (no diamond ¯\\_(ツ)_/¯)
 - Dynamic casting
 - No macros, external dependencies :)

[1] https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html

---

## How to use

Add `-fno-rtti` to your compile options in order to disable C++'s build in RTTI.

The `RTTI::Type` class describes an interface for performing RTTI checks and type casting from which the `RTTI::Base` class virtually derives to provides the implementation for the highest member of the class hierarchy. The `RTTI::Extends` class template provides the implementation for classes derived from `RTTI::Base`. Both the `RTTI::Base` and `RTTI::Extends` use the “Curiously Recurring Template Idiom”, taking the class being defined as its first template argument and the parent class as the second argument. Note that `RTTI::Extends` support multiple inheritance by allowing to pass a variadic number of parent classes. All parent classes passed need to be derived from either a `RTTI::Base` or `RTTI::Extends` in this case. It is possible to mixin types that are not part of an RTTI hierarchy, but this needs to be done outside of the template arguments.

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

    if (shape->is<Circle>()) {
        std::cout << "Yep, the shape is a circle!" << std::endl;
    }

    if (auto circle = shape->cast<Circle>()) {
        std::cout << "Woot, we have a circle! \0/" << std::endl;
    }

    if (shape->cast<Square>() == nullptr) {
        std::cout << "No, it is not a square... :(" << std::endl;        
    }

    if (auto base = shape->cast<SpecialBase>()) {
        std::cout << "The shape can also be cast to the special base class." << std::endl;
    }

    return 0;
}

```

Note that the `RTTI::TypeInfo<T>::Id()` method can also be used to identify any other types not part of an RTTI hierarchy, for example a very basic interface and implementation of a variant type:

```c++
struct AnyVariant {
    virtual ~AnyVariant() {}
    virtual RTTI::TypeId valueTypeId() const noexcept =0;
};

template<typename T>
struct Variant : AnyVariant {
    T value;

    virtual RTTI::TypeId valueTypeId() const noexcept override {
        return RTTI::TypeInfo<T>::Id();
    }
};

void testValueTypeId() {
    Variant<int> v;
    assert(v.valueTypeId() == TypeInfo<int>::Id());
    assert(v.valueTypeId() != TypeInfo<bool>::Id());
}
```

## Benchmark Results

```
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

## Contribute

Found a bug/mistake or have any other means you want to contribute? Feel free to open an issue or pull request! :)
