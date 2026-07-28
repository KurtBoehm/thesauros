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

# Try CMake’s OpenMP module first
find_package(OpenMP COMPONENTS CXX)

# Figure out what to link for OpenMP
set(THESAUROS_OPENMP_TARGET "")

if(TARGET OpenMP::OpenMP_CXX)
  # Normal case: CMake found OpenMP and created the imported target
  set(THESAUROS_OPENMP_TARGET OpenMP::OpenMP_CXX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  # Meson-like fallback for Apple Clang: `-Xpreprocessor -fopenmp` + `libomp`

  # User-configurable OpenMP runtime prefix
  set(THESAUROS_OPENMP_ROOT "" CACHE PATH "Prefix where libomp is installed")

  # Collect candidate prefixes
  set(_omp_prefixes "")

  if(THESAUROS_OPENMP_ROOT)
    list(APPEND _omp_prefixes "${THESAUROS_OPENMP_ROOT}")
  else()
    # If Homebrew is available, ask it for libomp’s prefix
    find_program(HOMEBREW_EXECUTABLE brew)
    if(HOMEBREW_EXECUTABLE)
      execute_process(
        COMMAND ${HOMEBREW_EXECUTABLE} --prefix libomp
        RESULT_VARIABLE _brew_result
        OUTPUT_VARIABLE
          _brew_prefix OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
      )
      if(_brew_result EQUAL 0 AND EXISTS "${_brew_prefix}")
        list(APPEND _omp_prefixes "${_brew_prefix}")
      endif()
    endif()
  endif()

  # Search for the library
  find_library(
    THESAUROS_OMP_LIB
    NAMES omp
    HINTS ${_omp_prefixes} PATH_SUFFIXES lib lib64
    REQUIRED
  )

  # Search for the header
  find_path(
    THESAUROS_OMP_INCLUDE_DIR
    NAMES omp.h
    HINTS ${_omp_prefixes} PATH_SUFFIXES include
    REQUIRED
  )

  add_library(thesauros_openmp INTERFACE)
  target_compile_options(thesauros_openmp INTERFACE -Xpreprocessor -fopenmp)
  target_link_libraries(thesauros_openmp INTERFACE ${THESAUROS_OMP_LIB})
  target_include_directories(
    thesauros_openmp
    INTERFACE ${THESAUROS_OMP_INCLUDE_DIR}
  )

  set(THESAUROS_OPENMP_TARGET thesauros_openmp)
else()
  message(FATAL_ERROR "OpenMP C++ support not found, but it is required.")
endif()

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
  INTERFACE ${PROJECT_NAME}_core ${THESAUROS_OPENMP_TARGET}
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
    Boost::preprocessor fmt::fmt ${THESAUROS_OPENMP_TARGET}
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
      fmt::fmt ${THESAUROS_OPENMP_TARGET}
  )

  target_link_libraries(${PROJECT_NAME}_resources INTERFACE ${PROJECT_NAME}_macos)
  target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}_macos)
endif()

if(THESAUROS_BUILD_TESTS)
  # Fetch cmop to add functions that add target options
  FetchContent_Declare(
    cmop
    GIT_REPOSITORY https://github.com/KurtBoehm/cmop.git
    GIT_TAG main
  )
  FetchContent_MakeAvailable(cmop)

  include(CTest)

  foreach(
    test_path
    IN
    ITEMS
    "algorithms/swap-or-equal"
    "algorithms/tiling"
    "charconv/charconv"
    "charconv/concat"
    "concepts/concepts"
    "containers/arrays"
    "containers/chunked-dynamic-array"
    "containers/dynamic-bitset"
    "containers/fixed-bitset"
    "containers/flat"
    "containers/multi-byte-integers"
    "containers/static-bitset"
    "execution/execution"
    "filesystem/tempfile"
    "format/format"
    "io/json"
    "io/serialization"
    "iterator/iterator-facades"
    "math/divisor"
    "math/factorization"
    "math/math"
    "math/tessellation"
    "quantity/quantity"
    "random/randomize-range"
    "ranges/ranges"
    "reflection/reflection"
    "static-ranges/static-ranges"
    "string/static-string"
    "types/empty"
    "types/primitives"
    "types/tuples"
    "types/type-seq"
    "types/value-tag"
    "utility/fancy-visit"
    "utility/index-position-wrapper"
    "utility/index-segmenter"
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
