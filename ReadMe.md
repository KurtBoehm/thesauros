# 🏦 Thesauros: A Treasury of C++23 Utilities

Thesauros (from Ancient Greek _θησαυρός_, “treasury”) is a header-only C++23 library by Kurt Böhm, providing a wide range of utilities that serve as a foundation for other projects.
Its layout loosely mirrors the C++ standard library: functionality is grouped into sub-libraries, each with its own umbrella header.

- `algorithms`: Tiled iteration (ascending/descending), parallelizable inclusive prefix sum, arbitrary-dimensional iteration.
- `charconv`: Character conversions: fixed-buffer number-to-string, string-to-integer parsing, `{fmt}`-friendly string escaping, Unicode decoding to code points.
- `concepts`: C++20 concepts, e.g. completeness checks, immutability-only access.
- `containers`:
  - Dynamic arrays with support for default initialization (unlike `std::vector`).
  - Arrays with mutable size backed by `std::array`.
  - Dynamic and static bitsets.
  - Flat maps and sets.
  - Chunked arrays with fixed-size or variable-size chunks (CSR-like layout without column indices).
  - Arbitrary-width integer containers: bit-packed into unsigned chunks or byte-based via `std::byte`.
- `execution`: Multithreading utilities: thread pools based on `std::thread` or OpenMP `parallel for`, plus thread affinity and scheduler helpers.
- `format`: `{fmt}` helpers: formatters for Thesauros types and simplified colour output.
- `functional`: Min/max function objects and a no-op function object.
- `io`: I/O abstractions built on `std::fread`/`std::fwrite` (avoiding the madness of stream flags/locales), plus JSON printing.
- `iterator`: Helpers for defining full-featured iterators with minimal boilerplate.
- `macropolis` (_Μακρόπολις_, a portmanteau of _macro_ and _Acropolis_): Preprocessor utilities: macros for classes/enums with support for static reflection, warning suppression, and inlining control.
- `math`:
  - Basic helpers: division rounded upward, `pow` with compile-time exponent, bit manipulation, etc.
  - Bounded/saturated arithmetic, overflow-/underflow-aware operations, safe integer casts.
  - Fast integer division/modulo with runtime-fixed divisors (inspired by https://arxiv.org/abs/1902.01961 and https://github.com/lemire/fastmod).
  - Integer factorization and uniform hypercube tessellation.
- `memory`: Memory tools, including a allocator using Linux’s Transparent Huge Pages.
- `quantity`: A `std::chrono::duration`-like quantity type with arbitrary units and related operations.
- `resources`: Resource and CPU information, including logical CPU lists with one logical CPU per physical core.
- `random`: Flexible Linear Congruential Generator and an on-the-fly range shuffler.
- `ranges`: Python-style `enumerate`, `range`, `reversed`, and `zip` ranges (increasingly superseded by C++23).
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

The project uses Meson and includes extensive tests:

```bash
meson setup <builddir>
meson test -C <builddir>
```

The provided `Makefile` offers convenience targets invoking `meson setup` with different optimization/debug options.

## Dependencies

Dependencies are provided as Meson subprojects (via [Tlaxcaltin](https://github.com/KurtBoehm/tlaxcaltin)), so no external packages need to be installed:

- [Boost.Preprocessor](https://github.com/boostorg/preprocessor): Macro utilities, foundational for Macropolis.
- [`{fmt}`](https://github.com/fmtlib/fmt): Core formatting and printing facility.
- [`options`](https://github.com/KurtBoehm/tlaxcaltin/blob/main/options/meson.build): Compiler options for stricter warnings and tuning.
- [`ankerl::unordered_dense::{map, set}`](https://github.com/martinus/unordered_dense): High-performance dense hash map/set used in factorization code.

## Platform Support

Supported and tested configurations:

- OS: Linux, macOS on Apple Silicon (tested on macOS Tahoe), Windows 11 (Windows 10 is expected to work).
- Architectures: x86-64 (Linux/Windows), AArch64 (Linux/macOS), RISC-V (Linux).
- Compilers: GCC ≥ 14, Clang ≥ 17.

Support for other operating systems or compilers (including MSVC) is not planned.

## Licences

Thesauros is licensed under the terms of the Mozilla Public Licence 2.0, which is provided in [`License`](License).

[`include/thesauros/charconv/unicode.hpp`](include/thesauros/charconv/unicode.hpp) is derived from code at https://bjoern.hoehrmann.de/utf-8/decoder/dfa/, which is licensed under the terms of the MIT licence; the corresponding licence is in [`include/thesauros/charconv/unicode-license`](include/thesauros/charconv/unicode-license).
