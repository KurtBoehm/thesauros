# Thesauros 🏦: A Collection of Diverse Utilities

Thesauros (Romanized from Ancient Greek _θησαυρός_, “treasury”) is a header-only C++20 library developed by Kurt Böhm which contains a wide range of facilities that form the basis for other projects.
Its structure is somewhat inspired by the standard library with its functionality split into sub-libraries, each with its own umbrella header.
These sub-libraries are, together with some highlights from each:

- `algorithms`: tiled iteration in ascending and descending order, parallel scan, multidimensional iteration.
- `charconv`:
  - Converting a number to a string with static memory.
  - Parsing a string containing an integer.
  - Escaping a string in a way useful for `{{fmt}}` formatters and similar use-cases.
  - Tools to decode a Unicode string into Unicode code points.
- `concepts`: whether a type is complete, whether it provides `const`-only access (which it true of `const` references as well, in contrast to `std::is_void`).
- `containers`:
  - Dynamic array types with support for default initialization, i.e. no initialization for primitive types (in contrast to e.g. `std::vector`).
  - An array type with dynamic size backed by an `std::array` allocation.
  - A chunked array type with fixed-size chunks and a dynamic array type which is split into variable-sized chunks (with the memory layout of a CSR matrix without the column indices).
  - A container containing integers with an arbitrary number of bits each (which are grouped into chunks represented by an unsigned integer type) as well as a container containing integers with an arbitrary number of bytes each.
  - Dynamic and static bitset as well as flat map and set types.
- `execution`: Thread pools backed by `std::thread` or OpenMP `parallel for` as well as functions which set the thread affinity and the scheduler.
- `format`: Utilities for using `{{fmt}}`, including support for many of Thesauros’ types and simplified colour printing.
- `functional`: Function objects computing the minimum or maximum.
- `io`: A file reader and writer based on `std::fread`/`std::fwrite` and friends, which avoid the madness of format flags and locales inherent to C++ stream-based I/O, as well as a JSON printer.
- `iterator`: Helpers for defining fully-featured iterators with very limited code.
- `macropolis`: Macropolis (_Μακρόπολις_, a portmanteau of _macro_ and _Acropolis_) is a set of preprocessor tools, including:
  - Macros to define classes and enumerations with compile-time reflection information, which can be used e.g. for serialization and deserialization.
  - Tools to flatten nested instances of `std::variant`.
  - Macros to disable warnings and to ensure/prevent inlining.
  - Macros to allow function objects returning `void` to be used in contexts in which the return value is optionally returned.
- `math`:
  - Simple functions such as safe bounded addition/subtraction, integer division rounded upwards, `pow` with a (compile-time constant) integer exponent, bit manipulation, etc.
  - Overflow-/underflow-aware and saturated arithmetic operations as well as safe integer casts.
  - Fast integer division and modulo for run-time fixed divisors inspired by https://arxiv.org/abs/1902.01961 and its implementation in https://github.com/lemire/fastmod.
  - Integer factorization.
  - Fast `sqrt` and `hypot`, exploiting SSE when possible, as well as fast `isfinite` and conversion into a finite value (by replacing NaN with 0).
- `memory`: A memory allocator exploiting Linux’s Transparent Huge Pages.
- `quantity`: A quantity class similar to `std::chrono::duration`, but with support for arbitrary units, as well as connected operations.
- `resources`: Utilities to get resource usage and CPU information, including a list of Linux CPUs with one “CPU” per physical core.
- `random`: A flexible implementation of a Linear Congruential Generator and a class that shuffles the integer values of a given range on the fly.
- `ranges`: Ranges emulating Python’s `enumerate`, `range`, `reversed`, and `zip`. This part becomes increasingly redundant with C++23.
- `static-ranges`: Static ranges which generalize `std::pair` and `std::tuple` with support for operations similar to C++20 ranges (including support for piping) and related operations.
- `string`: Compile-time strings, strings backed by `std::array`, and some character tools.
- `test`: Some assertion utilities as well as functions to check whether two containers or strings are equal with human-readable reports.
- `types`:
  - Fixed-size integers and floating-point numbers with Rust-style names, including `f16` and `i128`/`u128`, literals for these types, as well as helpers to get an integer type with a desired number of bytes.
  - Type and value tags to ease the use of template (member) functions.
  - Querying the (demangled) name of a type.
  - A type sequence type, with tools to work with them.
  - Type traits for the union/intersection of integer types.
  - Type traits to allow template types that can be `void` to be stored (by replacing them with an empty type).

While most components of Thesauros are written in standard C++20, some rely on Linux-specific functionality (especially in `execution` and `memory`) or GCC extensions (e.g. `THES_ALWAYS_INLINE`).
While some of these could be generalized to other platforms and compilers, this has not been a priority for the author, who only strives for compatibility with recent versions of GCC and Clang on Linux.

## Building

Thesauros uses the Meson build system and includes a fairly extensive set of tests.
These can be run by executing `meson setup -C <build directory>` followed by `meson test -C <build directory>`.
`Makefile` contains targets for calling `meson setup` with different optimization and debugging settings.

## Licences

Thesauros is licenced under the terms of the Mozilla Public Licence 2.0, which is provided in [`License`](License).
The file [`include/thesauros/charconv/unicode.hpp`](include/thesauros/charconv/unicode.hpp) is based on code provided on [https://bjoern.hoehrmann.de/utf-8/decoder/dfa/](https://bjoern.hoehrmann.de/utf-8/decoder/dfa/), which is licenced under the terms of the MIT licence as provided in [`include/thesauros/charconv/unicode-license`](include/thesauros/charconv/unicode-license).
