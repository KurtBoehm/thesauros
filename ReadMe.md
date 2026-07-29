# 🏦 Thesauros: A Treasury of C++23 Utilities

[![CI](https://github.com/KurtBoehm/thesauros/actions/workflows/ci.yml/badge.svg)](https://github.com/KurtBoehm/thesauros/actions/workflows/ci.yml)

Thesauros (from Ancient Greek _θησαυρός_, “treasury”) is a header-only C++23 library by Kurt Böhm, providing a wide range of utilities that serve as a foundation for other projects.
Its layout loosely mirrors the C++ standard library: functionality is grouped into sub-libraries, each with its own umbrella header.

- `algorithms`: Tiled iteration (ascending/descending), parallelizable inclusive prefix sum, arbitrary-dimensional iteration.
- `charconv`: Character conversions: fixed-buffer number-to-string, string-to-integer parsing, `{fmt}`-friendly string escaping, Unicode decoding to code points, and `cat`, which builds strings without depending on `{fmt}`.
- `concepts`: C++20 concepts, e.g. completeness checks, immutability-only access.
- `containers`:
  - Dynamic arrays with support for default initialization (unlike `std::vector`).
  - Arrays with mutable size backed by `std::array`.
  - Dynamic and static bitsets.
  - Flat maps and sets.
  - Chunked arrays with fixed-size or variable-size chunks (CSR-like layout without column indices).
  - Arbitrary-width integer containers: bit-packed into unsigned chunks or byte-based via `std::byte`.
- `execution`: Multithreading utilities: thread pools based on `std::thread` or OpenMP `parallel for`, plus thread affinity and scheduler helpers.
- `filesystem`: Temporary directories with RAII cleanup.
- `format`: `{fmt}` helpers: formatters for Thesauros types and simplified colour output.
- `functional`: Min/max function objects and a no-op function object.
- `io`: I/O abstractions built on `std::fread`/`std::fwrite` (avoiding the madness of stream flags/locales), plus JSON printing and binary serialization of the containers.
- `iterator`: Helpers for defining full-featured iterators with minimal boilerplate.
- `literals`: User-defined literals for the fixed-size integer types, floating-point types, and index tags. As in the standard library, `thes::literals` is an inline namespace holding one inline sub-namespace per group, so `using namespace thes::literals;` brings in all of them and `using namespace thes::literals::integer_literals;` just one.
- `macropolis` (_Μακρόπολις_, a portmanteau of _macro_ and _Acropolis_): Pure preprocessor utilities with no dependencies on the rest of the library: warning suppression, inlining control, platform detection, and `void`-handling macros.
- `math`:
  - Basic helpers: division rounded upward, `pow` with compile-time exponent, bit manipulation, etc.
  - Bounded/saturated arithmetic, overflow-/underflow-aware operations, safe integer casts.
  - Fast integer division/modulo with runtime-fixed divisors (inspired by https://arxiv.org/abs/1902.01961 and https://github.com/lemire/fastmod).
  - `thes::cmath`: `constexpr` counterparts of `<cmath>` functions that the standard library does not yet provide as such.
  - Integer factorization and uniform hypercube tessellation.
- `memory`: Memory tools, including a allocator using Linux’s Transparent Huge Pages.
- `quantity`: A `std::chrono::duration`-like quantity type with arbitrary units and related operations.
- `resources`: Resource and CPU information, including logical CPU lists with one logical CPU per physical core.
- `random`: Flexible Linear Congruential Generator and an on-the-fly range shuffler.
- `ranges`: Range types in `thes::ranges` and the factories producing them in `thes::views`, mirroring `std::ranges`/`std::views`: `views::indices` (a sized index range), `views::indices_n`, `views::enumerate`, and `views::cartesian_product`.
- `reflection`: Static reflection built on Macropolis, in `thes::reflect`: macros defining classes and enums along with the type information describing them, plus type flattening and serial names.
- `static-ranges`: Static ranges generalizing `std::pair`/`std::tuple`, range-like operations, and piping support.
- `string`: Compile-time strings, `std::array`-backed strings, and character utilities.
- `test`: Assertion helpers and container/string equality checks with human-readable reports.
- `types`:
  - Fixed-size integer/floating types with Rust-style names (`f16`, `i64`, `u128`, etc.), literals, and helpers for integer types with a given number of bytes.
  - Type/value tags to simplify templated interfaces.
  - Type sequences and associated utilities.
  - Integer-type union/intersection traits.
  - Traits to store `void`-capable template parameters via substitution with empty types.
  - Demangled type-name querying.
- `utility`:
  - Descriptors for arbitrary-byte integers.
  - Space-saving optional type using a reserved sentinel value.
  - Work-segmentation types for evenly partitioning integers (e.g. across threads).
  - Map with compile-time constant keys and runtime values.
  - Literal tuple type usable as a non-type template parameter.
  - Extended `std::optional` with C++23-style operations.
  - “Fancy visit”: `std::visit`-like dispatch for `std::variant` and non-variant arguments.
  - An empty type.

## Building

Thesauros is header-only on all platforms except macOS when `use_iokit` is enabled, which builds a small library because IOKit headers introduce aliases (such as `Size`) that conflict with Thesauros naming.

Meson and CMake are both fully supported: they build the same targets, fetch the same
dependency versions, expose the same per-group dependencies, and run the same tests.
CI exercises both.
The tests are laid out with one directory per sub-library under `test/`, registered as one Meson suite (or one CTest name prefix) per sub-library.

### Meson

```bash
meson setup <builddir> -Dbuild_tests=true
meson test -C <builddir>
```

A single sub-library’s tests can be run on their own:

```bash
meson test -C <builddir> --suite containers
```

CI additionally runs the whole test suite under the address and undefined behavior sanitizers, which is reproduced by adding `-Db_sanitize=address,undefined` to `meson setup`.

The provided `Makefile` offers convenience targets invoking `meson setup` with different optimization/debug options.

### CMake

```bash
cmake -S . -B <builddir> -DTHESAUROS_BUILD_TESTS=ON
cmake --build <builddir>
ctest --test-dir <builddir>
```

The options mirror the Meson ones: `THESAUROS_BUILD_TESTS` corresponds to `build_tests` and
`THESAUROS_USE_IOKIT` to `use_iokit`.
Dependencies are fetched with `FetchContent` rather than provided as subprojects, pinned to the same versions as the Meson wraps.

Consumers can fetch it:

```cmake
FetchContent_Declare(
  thesauros
  GIT_REPOSITORY https://github.com/KurtBoehm/thesauros.git
  GIT_TAG main
)
FetchContent_MakeAvailable(thesauros)

target_link_libraries(my_target PRIVATE thesauros::thesauros)
```

## Dependencies

Dependencies are provided as Meson subprojects (via [Tlaxcaltin](https://github.com/KurtBoehm/tlaxcaltin)), so no external packages need to be installed:

- [Boost.Preprocessor](https://github.com/boostorg/preprocessor): Macro utilities, foundational for Macropolis and Reflection.
- [`{fmt}`](https://github.com/fmtlib/fmt): Formatting and printing, used only by `format` and `test`.
- OpenMP: used only by the OpenMP thread pool in `execution`.
- [`options`](https://github.com/KurtBoehm/tlaxcaltin/blob/main/options/meson.build): Compiler options for stricter warnings and tuning (Meson only).

Most sub-libraries need none of these, so the dependency is declared per group rather than as a single monolith:

| Meson                     | CMake                  | Adds                                  | Covers                                                                                                                                                                      |
| ------------------------- | ---------------------- | ------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `thesauros_core_dep`      | `thesauros::core`      | nothing beyond the standard library   | `algorithms`, `charconv`, `concepts`, `containers`, `filesystem`, `functional`, `iterator`, `literals`, `math`, `memory`, `quantity`, `random`, `ranges`, `string`, `types` |
| `thesauros_macros_dep`    | `thesauros::macros`    | Boost.Preprocessor                    | `macropolis`, `reflection`, `io`, `static-ranges`, `utility`                                                                                                                |
| `thesauros_execution_dep` | `thesauros::execution` | OpenMP                                | `execution`                                                                                                                                                                 |
| `thesauros_resources_dep` | `thesauros::resources` | IOKit and the built library, on macOS | `resources`                                                                                                                                                                 |
| `thesauros_format_dep`    | `thesauros::format`    | `{fmt}`                               | `format`, `test`                                                                                                                                                            |
| `thesauros_dep`           | `thesauros::thesauros` | all of the above                      | everything                                                                                                                                                                  |

The `core dependency` test, present in both build systems, compiles every core sub-library with no external include directories at all to validate the first column.

## Platform Support

Supported and tested configurations:

- OS: Linux, macOS on Apple Silicon (tested on macOS Tahoe), Windows 11 (Windows 10 is expected to work).
- Architectures: x86-64 (Linux/Windows), AArch64 (Linux/macOS), RISC-V (Linux).
- Compilers: GCC ≥ 14, Clang ≥ 21.
  Clang 21 is required for its support of recent language features used throughout the library, deducing `this` above all.

Support for other operating systems or compilers (including MSVC) is not planned.
On Windows the build goes through MSYS2’s MinGW-w64 toolchain, since the thread-affinity code uses winpthreads.

CI builds each push and pull request in both a debug and an optimized configuration. Meson covers:

|         | x86-64                   | AArch64          |
| ------- | ------------------------ | ---------------- |
| Linux   | GCC 14, GCC 15, Clang 21 | GCC 14, Clang 21 |
| macOS   | —                        | Apple Clang      |
| Windows | MinGW GCC                | —                |

CMake is built and tested on one entry per platform, so that it cannot drift from Meson unnoticed.
Both minimum compiler versions are exercised deliberately, to keep constructs newer than the documented floor from creeping in.

macOS is supported on Apple Silicon only, and RISC-V has no hosted runner and is not covered.

## Licences

Thesauros is licensed under the terms of the Mozilla Public Licence 2.0, which is provided in [`License`](License).

[`include/thesauros/charconv/unicode.hpp`](include/thesauros/charconv/unicode.hpp) is derived from code at https://bjoern.hoehrmann.de/utf-8/decoder/dfa/, which is licensed under the terms of the MIT licence; the corresponding licence is in [`include/thesauros/charconv/unicode-license`](include/thesauros/charconv/unicode-license).
