# Thesauros 🏦: A Collection of Diverse Utilities

Thesauros (Romanized from Ancient Greek _θησαυρός_, “treasury”) is a header-only C++20 library developed by Kurt Böhm which contains a wide range of facilities that form the basis for other projects.
Its structure is somewhat inspired by the standard library with its functionality split into sub-libraries, each with its own umbrella header:

- `algorithms`: Operations including tiled iteration in ascending/descending order, parallelizable inclusive prefix sum, and arbitrary-dimensional iteration.
- `charconv`: Conversions to/from character stings, including converting a number to a string with static memory, parsing a string to an integer, escaping a string (in a way useful for `{{fmt}}` formatters), and decoding a Unicode string into Unicode code points.
- `concepts`: C++20 concepts, including ones describing whether a type is complete and whether it only provides immutable access.
- `containers`: A collection of general-purpose and more general containers, including:
  - Dynamic array types with support for default initialization (in contrast to e.g. `std::vector`).
  - An array type with mutable size backed by a statically-sized allocation using `std::array`.
  - Dynamic and static bitset types.
  - Flat map and set types.
  - Chunked array types with fixed-size chunks (using a flat array and simple index computations) or variable-sized chunks (with the memory layout of a CSR matrix without column indices).
  - Containers containing integers with an arbitrary number of bits each (which are grouped into chunks represented by an unsigned integer type) or integers with an arbitrary number of bytes each (backed by an array of `std::byte`).
- `execution`: Tools for efficient computations, especially using multithreading, including thread pools backed by `std::thread` or OpenMP `parallel for` as well as functions which set the thread affinity and the scheduler.
- `format`: Utilities for using `{{fmt}}`, including formatters for many of Thesauros’ types and simplified colour printing.
- `functional`: Function objects computing the minimum or maximum as well as a no-op function object.
- `io`: I/O tools including a file reader and a file writer based on `std::fread`/`std::fwrite` and friends, which avoid the madness of format flags and locales inherent to C++ stream-based I/O, as well as a JSON printer.
- `iterator`: Helpers for defining fully-featured iterators with very limited code.
- `macropolis`: Macropolis (_Μακρόπολις_, a portmanteau of _macro_ and _Acropolis_) is a set of preprocessor-based tools, including macros to define classes and enumerations with information enabling compile-time reflection and macros which disable warnings or ensure/prevent inlining.
- `math`: A collection of mathematical functions, including:
  - Simple functions such as integer division rounded upwards, `pow` with a (compile-time constant) integer exponent, bit manipulation, etc.
  - Fast `sqrt` and `hypot`, exploiting SSE when possible, as well as fast `isfinite` and conversion into a finite value (by replacing NaN with 0).
  - Bounded addition/subtraction, overflow-/underflow-aware arithmetic, saturated arithmetic operations, and safe integer casts.
  - Fast integer division and modulo for run-time fixed divisors inspired by https://arxiv.org/abs/1902.01961 and its implementation in https://github.com/lemire/fastmod.
  - Integer factorization and tessellating a hypercube with integer dimensions equidistantly in each dimension.
- `memory`: Memory tools including a memory allocator that exploits Linux’s Transparent Huge Pages.
- `quantity`: A quantity class similar to `std::chrono::duration` with support for arbitrary units and connected operations.
- `resources`: Utilities to get resource usage and CPU information, including a list of logical CPUs with one “CPU” per physical core.
- `random`: A flexible implementation of a Linear Congruential Generator and a class that shuffles the integer values of a given range on the fly.
- `ranges`: Ranges emulating Python’s `enumerate`, `range`, `reversed`, and `zip`. This part becomes increasingly redundant with C++23.
- `static-ranges`: Static ranges which generalize `std::pair` and `std::tuple` with support for operations similar to C++20 ranges (including support for piping), as well as a collection of related operations.
- `string`: Compile-time strings, strings backed by `std::array`, and some character tools.
- `test`: Some assertion utilities as well as functions to check whether two containers or strings are equal with human-readable reports.
- `types`: A collection of types and related utilities, including:
  - Fixed-size integers and floating-point numbers with Rust-style names, including `f16` and `i128`/`u128`, literals for these types, as well as helpers to get an (unsigned) integer type with a desired number of bytes.
  - Type and value tag types to ease the use of template (member) functions.
  - A type sequence type, with tools to work with them.
  - Type traits for the union/intersection of integer types.
  - Type traits to allow template types that can be `void` to be stored (by replacing them with an empty type).
  - Querying the (demangled) name of a type.
- `utility`: A catch-all for assorted utilities, including:
  - A type representing information about an integer with an arbitrary number of bytes.
  - An optional type which uses a given value of the underlying type to represent a missing value to save space.
  - Types that segment a given number of integers into a given number of segments, e.g. for a fixed work distribution across threads.
  - A map type consisting of compile-time constant keys and run-time values.
  - A tuple type which is a literal type, allowing it to be used as a non-type template parameter.
  - An extended `std::optional` with some operations that have been added in C++23.
  - A “fancy visit” function similar to `std::visit`, which handles both `std::variant` and other argument types appropriately.
  - An empty type.

While most components of Thesauros are written in standard C++20, some rely on Linux-specific functionality (especially in `execution` and `memory`) or GCC extensions (e.g. `THES_ALWAYS_INLINE`).
While some of these could be generalized to other platforms and compilers, this has not been a priority for the author, who only strives for compatibility with recent versions of GCC and Clang on Linux.

## Building

Thesauros uses the Meson build system and includes a fairly extensive set of tests.
These can be run by executing `meson setup -C <build directory>` followed by `meson test -C <build directory>`.
`Makefile` contains targets for calling `meson setup` with different optimization and debugging settings.

## Licences

Thesauros is licenced under the terms of the Mozilla Public Licence 2.0, which is provided in [`License`](License).
The file [`include/thesauros/charconv/unicode.hpp`](include/thesauros/charconv/unicode.hpp) is based on code provided on [https://bjoern.hoehrmann.de/utf-8/decoder/dfa/](https://bjoern.hoehrmann.de/utf-8/decoder/dfa/), which is licenced under the terms of the MIT licence as provided in [`include/thesauros/charconv/unicode-license`](include/thesauros/charconv/unicode-license).
