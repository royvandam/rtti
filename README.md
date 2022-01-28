# Open-hierarchy custom RTTI framework for C++17 and up.

If you have ever attempted to use the C++'s build in RTTI on a resource constrained (embedded) system you will most likely have noticed it is massively inefficient. Hence this implementation of a hand-rolled form of RTTI which is much more efficient and flexible, although it requires a bit more work from you as a class author. The current implementation supports the following features:

 - Compiletime (stable) ID generation based on the FNV1a hash of the type signature
 - Multiple inheritance, including virtual
 - Full dynamic casting support
 - Parent constructors are accessible
 - No external dependencies, single header
 - Static asserts on the parents passed to TypeInfo structure.
 - Works on bare-metal systems
 - Currently supports GCC/Clang based compilers
 - One convenience marco ¯\\_(ツ)_/¯

Note: This project was initially inspired by open-hierarchy examples in the guidelines defined for RTTI by the LLVM project<sup>[1]</sup>. However this solution has one major drawback which is that the parent constructors are no longer accessible given that RTTI classes are injected in between the parent and child. The initial (working) implementation based on this design is still available for reference under git tag `llvm-style-inheritance`.

[1] https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html

---

## How to use

 - Add `-fno-rtti` to your compile options in order to disable C++'s build in RTTI. (Optional, as it can work in conjunction with native RTTI)
 - The `RTTI::Enable` class the describes the abstract interface for performing runtime RTTI checks and type casing and is to be virtually derived from by the highest member(s) in the class hierarchy. For each type part of the hierarchy the `RTTI_DECLARE_TYPEINFO(Type, Parents...)` macro should be called to define an alias to `RTTI::TypeInfo` and overload the virtual methods of the interface described by `RTTI::Enable`.
 - The `RTTI::TypeInfo` class defines the type information and a describes the static interface for performing RTTI checks and type casting. It uses the “Curiously Recurring Template Idiom”, taking the class being defined as its first template argument and optionally the parent classes as the arguments there after.

### Basic example:

```c++
struct Shape : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(Shape);
};

struct Square : Shape {
    RTTI_DECLARE_TYPEINFO(Square, Shape);
};

struct OtherParent : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(OtherParent);
}

struct Circle : Shape, OtherParent {
    RTTI_DECLARE_TYPEINFO(Circle, Shape, OtherParent);
};

int main() {
    Circle c;
    Shape* shape = &c;

    if (shape->is<Circle>()) {
        std::cout << "Yes, the shape is a circle!" << std::endl;
    }

    if (shape->cast<Square>() == nullptr) {
        std::cout << "No, it was not a square... :(" << std::endl;        
    }

    if (auto circle = shape->cast<Circle>()) {
        std::cout << "Woot, we have the circle back! \\0/" << std::endl;
    }

    OtherParent *p = &c;
    if (auto s = p->cast<Shape>()) {
        std::cout << "Pointer offsets are take into account for multiple inheritance hierarchies." << std::endl;
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
Load Average: 0.48, 0.26, 0.17
------------------------------------------------------------
Benchmark                  Time             CPU   Iterations
------------------------------------------------------------
NativeDynamicCast        133 ns          133 ns      5252735
RttiDynamicCast         6.08 ns         6.08 ns    114135771
```

## Contribute

Found a bug/mistake or have any other means you want to contribute? Feel free to open an issue or pull request! :)
