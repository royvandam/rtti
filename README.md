# Open-hierarchy custom RTTI framework for C++17 and up.

Compiler-provided RTTI can be expensive on resource-constrained embedded systems. This framework provides an alternative in which class authors explicitly describe their inheritance hierarchies. The current implementation supports the following features:

 - Compile-time ID generation using deterministic type-name hashes by default, or optional process-local pointers
 - Multiple inheritance, including virtual
 - Checked pointer casts, including downcasts and cross-casts
 - Parent constructors are accessible
 - Header-only library with two headers and no external library dependencies
 - Static asserts on the parents passed to TypeInfo structure.
 - Works on bare-metal systems
 - Supports GCC/Clang based compilers and MSVC; uses `std::source_location` when available in C++20 and up
 - Two convenience macros for runtime and non-polymorphic type declarations

Note: This project was initially inspired by open-hierarchy examples in the guidelines defined for RTTI by the LLVM project<sup>[1]</sup>. However this solution has one major drawback which is that the parent constructors are no longer accessible given that RTTI classes are injected in between the parent and child. An initial implementation of the RTTI library was based on this design and is still available for reference under git tag `llvm-style-inheritance`.

[1] https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html

---

## How to use

### Add the library to your project

The library requires C++17 or later. With this repository checked out at
`external/rtti`, add the following to your application's `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(example LANGUAGES CXX)

set(BUILD_TESTS OFF CACHE BOOL "Build RTTI tests" FORCE)
set(BUILD_BENCHMARK OFF CACHE BOOL "Build RTTI benchmarks" FORCE)
add_subdirectory(external/rtti)

add_executable(example main.cc)
target_link_libraries(example PRIVATE rtti)
```

Save the basic example below as `main.cc`, then configure and build:

```sh
cmake -S . -B build
cmake --build build
```

The `rtti` interface target supplies the include directory, the C++17 minimum,
and the selected type-ID configuration. Tests and benchmarks default to `ON`;
disabling both avoids downloading their dependencies when consuming the library.

For direct header use, copy both `include/rtti.hh` and `include/hash.hh` into
the same include directory, then include `<rtti.hh>`. For example, with the
repository at `external/rtti`, GCC or Clang can build `main.cc` directly:

```sh
c++ -std=c++17 -Iexternal/rtti/include main.cc -o example
```

### Declare runtime types

 - Compiler-provided RTTI is optional. Disable it with `-fno-rtti` on GCC/Clang or `/GR-` on MSVC, or leave it enabled to use both implementations together.
 - Virtually derive the root class or classes of your hierarchy from `RTTI::Enable`, which provides runtime type checks and pointer casts.
 - Add `RTTI_DECLARE_TYPEINFO(Type, Parents...)` to each participating class, passing the class itself and its direct parent types. This exposes a public `TypeInfo` alias and overrides the virtual RTTI methods.
 - Each listed parent must expose its own `TypeInfo`. Polymorphic parents must derive from `RTTI::Enable`; non-polymorphic parents can use `RTTI_DECLARE_TRIVIAL`, as shown below. Checks and casts follow the declared parent lists.

### Basic example

```c++
#include <rtti.hh>
#include <iostream>

struct Shape : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(Shape);
};

struct Square : Shape {
    RTTI_DECLARE_TYPEINFO(Square, Shape);
};

struct OtherParent : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(OtherParent);
};

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
        std::cout << "Pointer offsets are taken into account for multiple inheritance hierarchies." << std::endl;
    }

    return 0;
}
```

Both declaration macros finish in `private:`. Add an explicit `public:` after
the macro before declaring public constructors, methods, or data members, even
in a `struct`. End each macro invocation with a semicolon.

Classes derived from `RTTI::Enable` are non-copyable and non-movable by default:
the base deletes move assignment, which also prevents implicit copying and
moving of derived objects. Account for this when storing objects by value.

### Non-polymorphic types and mixed hierarchies

`RTTI_DECLARE_TRIVIAL(Type, Parents...)` exposes the static `TypeInfo` API
without adding virtual functions or deriving from `RTTI::Enable`. Use it for
non-polymorphic types whose parents are also non-polymorphic. It preserves
otherwise available copy and move operations. Such types can also be parents
of a runtime-enabled class:

```c++
#include <rtti.hh>
#include <cassert>
#include <type_traits>

struct Position {
    RTTI_DECLARE_TRIVIAL(Position);

public:
    int x = 0;
};

struct Point : Position {
    RTTI_DECLARE_TRIVIAL(Point, Position);
};

static_assert(Point::TypeInfo::Is(Position::TypeInfo::Id()));
static_assert(!std::is_polymorphic_v<Point>);
static_assert(std::is_copy_constructible_v<Point>);
static_assert(std::is_move_constructible_v<Point>);

struct Shape : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(Shape);
};

struct PositionedShape : Shape, Point {
    RTTI_DECLARE_TYPEINFO(PositionedShape, Shape, Point);
};

int main() {
    PositionedShape shape;
    RTTI::Enable& object = shape;
    assert(object.is<Position>());
    assert(object.cast<Position>() == static_cast<Position*>(&shape));
}
```

### API reference

For an RTTI-enabled object `obj`, the runtime API is:

| Operation | Result |
| --- | --- |
| `obj.typeId()` | ID of the most-derived registered type. Compare with `T::TypeInfo::Id()` for an exact-type check. |
| `obj.is<T>()` | Whether `T` is the object's registered type or a declared ancestor, including indirect ancestors. |
| `obj.isById(id)` | The same inheritance check using a type ID. |
| `obj.cast<T>()` | Adjusted `T*`, or `T const*` for a const object; `nullptr` if the target is absent or ambiguous. |
| `obj.cast<void>()` | Address of the most-derived registered object as `void*`, or `void const*` for a const object. |
| `obj.cast<const void>()` | Address of the most-derived registered object as `void const*`. |

These operations are `noexcept`. Casts accept a target type such as `Shape`,
not `Shape*`, and preserve constness; `cast<const Shape>()` is also supported.
They return pointers only, with no reference-cast overload. When calling through
a pointer, check that the source pointer is non-null before invoking a member.

The static API is available without an object:

| Operation | Result |
| --- | --- |
| `T::TypeInfo::Id()` / `T::TypeInfo::Name()` | Compile-time ID / compiler-generated name of a declared type. |
| `T::TypeInfo::Is(id)` | Compile-time check for the type itself or a declared ancestor. |
| `T::TypeInfo::DynamicCast(id, ptr)` | Static hierarchy traversal returning an adjusted `void const*` or `nullptr`; pass a pointer to the declared type `T`. Can be evaluated at compile time when the pointer conversions permit it. |
| `RTTI::TypeInfo<T>::Id()` / `RTTI::TypeInfo<T>::Name()` | ID / name for any type, including types outside an RTTI hierarchy. Top-level `const` and references are removed. |
| `RTTI::TypeName<T>()` | Compiler-generated type name preserving qualifiers and references. |

Use the class's `T::TypeInfo` alias for hierarchy traversal: it includes the
declared parents, whereas `RTTI::TypeInfo<T>` alone does not.

### Casting limitations

Each derived class must declare its own RTTI information to be identified as
that type. A class that only inherits its parent's RTTI overrides is reported
as that parent, including for `typeId()` and `cast<void>()`.

Casts resolve the target from the most-derived registered type. In a
non-virtual diamond containing two `Base` subobjects, `obj.is<Base>()` can
return `true` while `obj.cast<Base>()` returns `nullptr` because the base is
ambiguous. This also applies when calling through a `Base*`: the cast does not
retain which base subobject the call came from. A successful inheritance check
therefore does not guarantee that a cast succeeds. Virtual diamonds with a
single shared base support casts to that base.

### Identify types outside a hierarchy

Note that the `RTTI::TypeInfo<T>::Id()` method can also be used to identify any other types not part of an RTTI hierarchy, for example a very basic interface and implementation of a variant type:

```c++
#include <rtti.hh>
#include <cassert>

struct AnyVariant {
    virtual ~AnyVariant() = default;
    virtual RTTI::TypeId valueTypeId() const noexcept = 0;
};

template<typename T>
struct Variant : AnyVariant {
    T value{};

    RTTI::TypeId valueTypeId() const noexcept override {
        return RTTI::TypeInfo<T>::Id();
    }
};

int main() {
    Variant<int> v;
    assert(v.valueTypeId() == RTTI::TypeInfo<int>::Id());
    assert(v.valueTypeId() != RTTI::TypeInfo<bool>::Id());
}
```

## Zephyr

When this repository is included as a Zephyr module, enable the framework in your application's `prj.conf`. For a C++17 application:

```conf
CONFIG_CPP=y
CONFIG_STD_CPP17=y
CONFIG_CPP_FAST_RTTI=y
```

C++20 and later standards are also supported. The framework provides its own type checks and pointer casts, so Zephyr's compiler-provided RTTI option (`CONFIG_CPP_RTTI`) can remain disabled.

## Type ID configuration

By default, `RTTI::TypeId` is a `std::uint32_t` containing the 32-bit FNV-1a hash of the type name. Hash IDs are deterministic for the same compiler-generated type name; different compilers or changes to those names can produce different IDs.

Hash IDs are not guaranteed to be unique. The library compares IDs without
detecting collisions or checking type names, so a collision can produce a false
type match or an incorrect cast pointer. Use pointer IDs when hash collisions
are unacceptable within a single process, subject to the shared-library
requirements below.

The FNV-1a multiplier has been corrected from `19777619` to `16777619`, and input bytes are now interpreted as unsigned. This changes hash IDs produced by earlier revisions. Rebuild all components exchanging RTTI objects together, and regenerate stored hash IDs when upgrading. Pointer IDs are unaffected.

To use process-local pointer IDs, configure CMake with `-DRTTI_USE_POINTER_TYPE_IDS=ON` and link consumers to the `rtti` target. For direct header use, define `RTTI_USE_POINTER_TYPE_IDS=1` before including `rtti.hh`, or pass `-DRTTI_USE_POINTER_TYPE_IDS=1` to the compiler. The default is `0` (hash IDs).

In pointer mode, `RTTI::TypeId` is `void const*`. Each type is identified by the address of a C++17 inline variable, with no external dependency or runtime registration. These IDs are only valid within the running program and must not be persisted or exchanged between processes. Shared-library boundaries require consistent symbol visibility for the type tags.

All translation units and libraries exchanging RTTI objects must use the same setting. Both modes support C++17, retain `TypeInfo<T>::Name()`, and use short-circuit traversal for dynamic casts.

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

## Development

Enter the development environment with `nix-shell`. It uses your configured
`nixpkgs` and provides GCC, CMake, Ninja, `clangd`, `clang-format`, `clang-tidy`,
Cppcheck, GDB, and Git.

```sh
nix-shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The shell selects Ninja and enables `build/compile_commands.json` for language
server support. The first build downloads and installs GoogleTest and Google
Benchmark into `build/external`, so it requires network access. Build once to
make their headers available to the language server.

CTest runs the complete suite both with and without compiler-provided RTTI.
Configuration also checks that invalid parent declarations fail to compile with
the expected diagnostics. The CI matrix covers GCC and Clang, C++17 and C++20,
both type-ID modes, unsigned `char`, and Clang sanitizers.

To test another configuration locally, use a separate build directory:

```sh
cmake -S . -B build/pointer-cxx20 -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_STANDARD=20 -DRTTI_USE_POINTER_TYPE_IDS=ON -DBUILD_BENCHMARK=OFF
cmake --build build/pointer-cxx20
ctest --test-dir build/pointer-cxx20 --output-on-failure
```

For sanitizers, configure with `CXX=clang++` and
`-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"`.
GCC 15.3 with UBSan rejects constexpr pointer-tag comparisons in pointer-ID
mode, including in a minimal program without this library; use Clang to test
that combination.

### VS Code, including Remote SSH

Install the recommended **clangd** (`llvm-vs-code-extensions.vscode-clangd`),
**CMake Tools** (`ms-vscode.cmake-tools`), and
[Workspace Shared Settings](https://github.com/bohdan-shulha/vs-wss)
(`b-shulha.wss`) extensions. For Remote SSH, install them on the remote host.
Open this project in a trusted workspace and run **Workspace Shared Settings:
Apply Workspace Settings** from the Command Palette, then reload the window.

The extension expands `.vscode/settings.workspace.json` into the local,
Git-ignored `.vscode/settings.json`. The helper enters this project's
`shell.nix` to resolve absolute paths for `clangd`, CMake, Ninja, and the
compilers. CMake also receives the shell's `PATH`, with temporary directories
set to `/tmp` so they remain valid after the helper's Nix shell exits. This
works when the VS Code extension host starts outside `nix-shell`, including
over SSH; opening `nix-shell` in an integrated terminal alone does not update
that host's environment.

Run **CMake: Configure**, then **CMake: Build** (or use the commands above).
`clangd` reads `build/compile_commands.json` and queries the resolved Nix C++
compiler for system header paths. C++ completion, navigation, diagnostics, and
formatting use `clangd`; the Microsoft C/C++ IntelliSense engine is disabled
in the shared settings to avoid duplicate diagnostics.

Keep shared settings in `settings.workspace.json`; command substitutions are
expanded by Workspace Shared Settings rather than VS Code itself. Reapply the
settings after changing `shell.nix` or updating Nix packages, reconfigure the
build, and run **clangd: Restart language server**.

## Contribute

Have you found a bug/mistake or any other proposal and want to contribute? Feel free to open an issue or pull request!
Happy coding!
