# Open-hierarchy custom RTTI framework for C++17 and up.

If you have ever attempted to use the C++'s build in RTTI on a resource constrained (embedded) system you will most likely have noticed it is massively inefficient. Hence this implementation of a hand-rolled form of RTTI which is much more efficient and flexible, although it requires a bit more work from you as a class author. The current implementation supports the following features:

 - Compiletime ID generation using deterministic type-name hashes by default, or optional process-local pointers
 - Multiple inheritance, including virtual
 - Full dynamic casting support
 - Parent constructors are accessible
 - No external dependencies, single header
 - Static asserts on the parents passed to TypeInfo structure.
 - Works on bare-metal systems
 - Supports GCC/Clang based compilers and MSVC; uses `std::source_location` when available in C++20 and up
 - One convenience marco ¯\\_(ツ)_/¯

Note: This project was initially inspired by open-hierarchy examples in the guidelines defined for RTTI by the LLVM project<sup>[1]</sup>. However this solution has one major drawback which is that the parent constructors are no longer accessible given that RTTI classes are injected in between the parent and child. An initial implementation of the RTTI library was based on this design and is still available for reference under git tag `llvm-style-inheritance`.

[1] https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html

---

## How to use

 - Add `-fno-rtti` to your compile options in order to disable C++'s build in RTTI. (Optional, as it can work in conjunction with native RTTI)
 - `RTTI::Enable` describes the abstract interface for performing runtime RTTI checks and type casing. It is to be virtually derived from by the highest member(s) in the class hierarchy.
 - For each type part of the hierarchy the `RTTI_DECLARE_TYPEINFO(Type, Parents...)` macro should be added after the opening brace to define a type alias to `RTTI::TypeInfo` structure and overload the virtual methods of the interface described by `RTTI::Enable`.
 - `RTTI::TypeInfo` holds the type information of each member and provides statis methods for performing RTTI checks and type casting. It uses the “Curiously Recurring Template Idiom”, taking the class being defined as its first template argument and optionally the parent classes as the arguments there after.

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
