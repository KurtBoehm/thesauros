# This file is part of https://github.com/KurtBoehm/thesauros.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.20)
project(thesauros LANGUAGES CXX)

# Build options
option(THESAUROS_BUILD_TESTS "Build tests" OFF)
option(THESAUROS_USE_IOKIT "Use macOS IOKit/CoreFoundation for CPU info" OFF)

# On macOS, _Static_assert is used in system headers, which GCC does not support
# in C++ mode
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND APPLE)
  add_compile_definitions(_Static_assert=static_assert)
endif()

# Ensure we build {fmt} as a static library and don’t pull in extra stuff
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_SHARED OFF CACHE BOOL "Build fmt as a shared library" FORCE)

include(FetchContent)

# Fetch cmop for cmop_find_openmp() (used right below) and, when building tests, the target-option
# functions.
FetchContent_Declare(
  cmop
  GIT_REPOSITORY https://github.com/KurtBoehm/cmop.git
  GIT_TAG main
)
FetchContent_MakeAvailable(cmop)

# thesauros_execution needs OpenMP for its OpenMP-based thread pool, and the umbrella target below
# needs it too, so it is required unconditionally rather than only when building tests.
cmop_find_openmp()

FetchContent_Declare(
  boost_preprocessor
  URL
    https://github.com/boostorg/preprocessor/archive/refs/tags/boost-1.90.0.tar.gz
  URL_HASH
    SHA256=4a951411d94423e74a4f1df5b1aa5d51996dbfbf01bec35bc51c53af4707bda3
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(boost_preprocessor)

FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/12.1.0.tar.gz
  URL_HASH
    SHA256=ea7de4299689e12b6dddd392f9896f08fb0777ac7168897a244a6d6085043fea
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(fmt)

# Main interface library
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

target_compile_definitions(
  ${PROJECT_NAME}
  INTERFACE "THES_USE_IOKIT=$<BOOL:${THESAUROS_USE_IOKIT}>"
)

# Meson: On Windows set WINVER/_WIN32_WINNT
if(WIN32)
  target_compile_definitions(
    ${PROJECT_NAME}
    INTERFACE WINVER=0x0A00 _WIN32_WINNT=0x0A00
  )
endif()

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Dependencies per group of sub-libraries, so that consumers only pay for what they include.
# These mirror the `thesauros_*_dep` objects declared in meson.build.

# Everything that needs nothing beyond the standard library: algorithms, charconv, concepts,
# containers, filesystem, functional, iterator, literals, math, memory, quantity, random,
# ranges, string and types.
add_library(${PROJECT_NAME}_core INTERFACE)
add_library(${PROJECT_NAME}::core ALIAS ${PROJECT_NAME}_core)
target_link_libraries(${PROJECT_NAME}_core INTERFACE ${PROJECT_NAME})

# macropolis, reflection and everything using their macros: io, static-ranges, utility.
add_library(${PROJECT_NAME}_macros INTERFACE)
add_library(${PROJECT_NAME}::macros ALIAS ${PROJECT_NAME}_macros)
target_link_libraries(
  ${PROJECT_NAME}_macros
  INTERFACE ${PROJECT_NAME}_core Boost::preprocessor
)

# execution uses OpenMP for its OpenMP-based thread pool.
add_library(${PROJECT_NAME}_execution INTERFACE)
add_library(${PROJECT_NAME}::execution ALIAS ${PROJECT_NAME}_execution)
target_link_libraries(
  ${PROJECT_NAME}_execution
  INTERFACE ${PROJECT_NAME}_core cmop::openmp
)

# resources needs IOKit, and the library built from it, only on macOS; the link is added below.
add_library(${PROJECT_NAME}_resources INTERFACE)
add_library(${PROJECT_NAME}::resources ALIAS ${PROJECT_NAME}_resources)
target_link_libraries(${PROJECT_NAME}_resources INTERFACE ${PROJECT_NAME}_core)

# format and test are the only sub-libraries reaching {fmt}; format also formats the reflection
# and resource types, so it inherits both of those.
add_library(${PROJECT_NAME}_format INTERFACE)
add_library(${PROJECT_NAME}::format ALIAS ${PROJECT_NAME}_format)
target_link_libraries(
  ${PROJECT_NAME}_format
  INTERFACE ${PROJECT_NAME}_macros ${PROJECT_NAME}_resources fmt::fmt
)

# The umbrella target keeps its historical meaning: everything.
target_link_libraries(
  ${PROJECT_NAME}
  INTERFACE
    Boost::preprocessor fmt::fmt cmop::openmp
)

# Optional library on macOS when THESAUROS_USE_IOKIT is true
if(APPLE AND THESAUROS_USE_IOKIT)
  add_library(${PROJECT_NAME}_macos STATIC src/resources/cpu-info-macos.cpp)

  target_compile_features(${PROJECT_NAME}_macos PRIVATE cxx_std_23)

  target_include_directories(
    ${PROJECT_NAME}_macos
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  target_compile_definitions(${PROJECT_NAME}_macos PRIVATE THES_USE_IOKIT=1)

  find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
  find_library(IOKIT_FRAMEWORK IOKit REQUIRED)

  target_link_libraries(
    ${PROJECT_NAME}_macos
    PRIVATE
      ${COREFOUNDATION_FRAMEWORK} ${IOKIT_FRAMEWORK} Boost::preprocessor
      fmt::fmt cmop::openmp
  )

  target_link_libraries(${PROJECT_NAME}_resources INTERFACE ${PROJECT_NAME}_macos)
  target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}_macos)
endif()

if(THESAUROS_BUILD_TESTS)
  # cmop was already fetched above for cmop_find_openmp(); it also provides the target-option
  # functions used below.
  include(CTest)

  foreach(
    test_path
    IN
    ITEMS
    "algorithms/sort-indices"
    "algorithms/swap-or-equal"
    "algorithms/tiling"
    "charconv/charconv"
    "charconv/concat"
    "concepts/concepts"
    "containers/array-policies"
    "containers/arrays"
    "containers/chunked-dynamic-array"
    "containers/dynamic-bitset"
    "containers/dynamic-buffer"
    "containers/fixed-bitset"
    "containers/flat"
    "containers/flat-map"
    "containers/flat-set"
    "containers/multi-bit-integers"
    "containers/multi-byte-integers"
    "containers/nested-dynamic-array"
    "containers/set-algorithms"
    "containers/static-bitset"
    "execution/execution"
    "execution/thread-pool"
    "filesystem/tempfile"
    "format/format"
    "format/formatters"
    "functional/functional"
    "io/file"
    "io/json"
    "io/serialization"
    "iterator/iterator-facades"
    "math/arithmetic"
    "math/divisor"
    "math/factorization"
    "math/integer-cast"
    "math/math"
    "math/overflow"
    "math/tessellation"
    "memory/byte-read"
    "quantity/quantity"
    "random/lcg"
    "random/randomize-range"
    "ranges/ranges"
    "reflection/reflection"
    "static-ranges/static-ranges"
    "static-ranges/views-and-sinks"
    "string/static-capacity-string"
    "string/static-string"
    "types/empty"
    "types/integer-traits"
    "types/primitives"
    "types/tuples"
    "types/type-seq"
    "types/type-transformations"
    "types/value-tag"
    "types/void-storage"
    "utility/check-variant"
    "utility/fancy-visit"
    "utility/index-position-wrapper"
    "utility/index-segmenter"
    "utility/multi-bit-reference"
    "utility/multi-size"
    "utility/optional"
    "utility/small-utilities"
    "utility/static-map"
    "utility/std-extensions"
  )
    string(REPLACE "/" "_" test_target_name ${test_path})
    set(test_target ${PROJECT_NAME}_${test_target_name})
    add_executable(${test_target} test/${test_path}.cpp)
    target_compile_features(${test_target} PRIVATE cxx_std_23)
    target_link_libraries(${test_target} PRIVATE ${PROJECT_NAME})
    cmop_add_warnings(${test_target})
    cmop_add_info(${test_target})
    add_test(NAME ${PROJECT_NAME}.${test_target_name} COMMAND ${test_target})
  endforeach()

  # Compiled against the include directory alone, mirroring the `core dependency` Meson test.
  add_executable(${PROJECT_NAME}_core_dep test/core-dep.cpp)
  target_compile_features(${PROJECT_NAME}_core_dep PRIVATE cxx_std_23)
  target_include_directories(
    ${PROJECT_NAME}_core_dep
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
  target_compile_definitions(
    ${PROJECT_NAME}_core_dep
    PRIVATE "THES_USE_IOKIT=$<BOOL:${THESAUROS_USE_IOKIT}>"
  )
  cmop_add_warnings(${PROJECT_NAME}_core_dep)
  add_test(NAME ${PROJECT_NAME}.core_dep COMMAND ${PROJECT_NAME}_core_dep)

  # The structural checks over the headers themselves, mirroring the Meson `check includes` test.
  find_package(Python3 COMPONENTS Interpreter)
  if(Python3_Interpreter_FOUND)
    add_test(
      NAME ${PROJECT_NAME}.check_includes
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/check-includes.py
    )
  endif()
endif()
