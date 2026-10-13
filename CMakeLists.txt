# This file is part of https://github.com/KurtBoehm/thesauros.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.21)
project(thesauros LANGUAGES CXX)

# Build options.
option(THESAUROS_BUILD_TESTS "Build tests" OFF)
option(THESAUROS_INSTALL "Generate install and export rules"
       ${PROJECT_IS_TOP_LEVEL}
)
option(THESAUROS_SYSTEM_BOOST "Use an installed Boost" OFF)

# On macOS, _Static_assert is used in system headers, which GCC does not support
# in C++ mode.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND APPLE)
  add_compile_definitions(_Static_assert=static_assert)
endif()

include(FetchContent)
include(GNUInstallDirs)

if(THESAUROS_SYSTEM_BOOST)
  find_package(Boost CONFIG REQUIRED)
  set(THESAUROS_BOOST_PP Boost::headers)
  set(THESAUROS_BUNDLED_BOOST OFF)
else()
  # Only download the sources: the non-existent SOURCE_SUBDIR keeps FetchContent
  # from adding Boost.Preprocessor’s own (non-exportable) CMake project.
  FetchContent_Declare(
    boost_preprocessor
    URL
      https://github.com/boostorg/preprocessor/archive/refs/tags/boost-1.90.0.tar.gz
    URL_HASH
      SHA256=4a951411d94423e74a4f1df5b1aa5d51996dbfbf01bec35bc51c53af4707bda3
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    SOURCE_SUBDIR none
  )
  FetchContent_MakeAvailable(boost_preprocessor)

  add_library(${PROJECT_NAME}_boost_pp INTERFACE)
  set_target_properties(
    ${PROJECT_NAME}_boost_pp
    PROPERTIES EXPORT_NAME boost_pp
  )
  target_include_directories(
    ${PROJECT_NAME}_boost_pp
    SYSTEM
    INTERFACE
      $<BUILD_INTERFACE:${boost_preprocessor_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/thesauros-vendor>
  )

  set(THESAUROS_BOOST_PP ${PROJECT_NAME}_boost_pp)
  set(THESAUROS_BUNDLED_BOOST ON)
endif()

# fmt: Prefer an installed fmt; if fetched, let it install itself next to
# Thesauros.
find_package(fmt 12 CONFIG QUIET)
if(NOT fmt_FOUND)
  # Plain cache defaults instead of `FORCE` so a parent project’s choice takes
  # precendence.
  set(FMT_DOC OFF CACHE BOOL "")
  set(FMT_TEST OFF CACHE BOOL "")
  set(FMT_INSTALL ${THESAUROS_INSTALL} CACHE BOOL "")
  set(FMT_SHARED OFF CACHE BOOL "Build fmt as a shared library")
  FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/12.1.0.tar.gz
    URL_HASH
      SHA256=ea7de4299689e12b6dddd392f9896f08fb0777ac7168897a244a6d6085043fea
    DOWNLOAD_EXTRACT_TIMESTAMP ON
  )
  FetchContent_MakeAvailable(fmt)
endif()

# The main interface library.
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

# Set Windows WINVER/_WIN32_WINNT on Windows.
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

# Dependencies per group of sub-libraries, so that consumers only pay for what
# they include.
# These mirror the `thesauros_*_dep` objects declared in meson.build.

# Everything that needs nothing beyond the standard library: algorithms,
# charconv, concepts, containers, execution, filesystem, functional, iterator,
# literals, math, memory, quantity, random, ranges, resources, string, and
# types.
add_library(${PROJECT_NAME}_core INTERFACE)
add_library(${PROJECT_NAME}::core ALIAS ${PROJECT_NAME}_core)
target_link_libraries(${PROJECT_NAME}_core INTERFACE ${PROJECT_NAME})

# argparse writes its help text and error messages with {fmt}, but needs nothing
# else.
add_library(${PROJECT_NAME}_argparse INTERFACE)
add_library(${PROJECT_NAME}::argparse ALIAS ${PROJECT_NAME}_argparse)
target_link_libraries(
  ${PROJECT_NAME}_argparse
  INTERFACE ${PROJECT_NAME}_core fmt::fmt
)

# macropolis, reflection and everything using their macros: io, static-ranges,
# utility.
add_library(${PROJECT_NAME}_macros INTERFACE)
add_library(${PROJECT_NAME}::macros ALIAS ${PROJECT_NAME}_macros)
target_link_libraries(
  ${PROJECT_NAME}_macros
  INTERFACE ${PROJECT_NAME}_core ${THESAUROS_BOOST_PP}
)

# format and test are the only sub-libraries reaching {fmt}; format also formats
# the reflection and resource types, so it inherits both of those.
add_library(${PROJECT_NAME}_format INTERFACE)
add_library(${PROJECT_NAME}::format ALIAS ${PROJECT_NAME}_format)
target_link_libraries(
  ${PROJECT_NAME}_format
  INTERFACE ${PROJECT_NAME}_macros fmt::fmt
)

# The umbrella target keeps its historical meaning: everything.
target_link_libraries(${PROJECT_NAME} INTERFACE ${THESAUROS_BOOST_PP} fmt::fmt)

foreach(component core argparse macros format)
  set_target_properties(
    ${PROJECT_NAME}_${component}
    PROPERTIES EXPORT_NAME ${component}
  )
endforeach()

if(THESAUROS_INSTALL)
  set(_cfgdir ${CMAKE_INSTALL_LIBDIR}/cmake/thesauros)

  install(
    TARGETS thesauros thesauros_core thesauros_argparse thesauros_macros
    thesauros_format
    EXPORT thesaurosTargets
  )
  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  if(THESAUROS_BUNDLED_BOOST)
    install(TARGETS ${PROJECT_NAME}_boost_pp EXPORT thesaurosTargets)
    install(
      DIRECTORY ${boost_preprocessor_SOURCE_DIR}/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thesauros-vendor
    )
  endif()

  install(EXPORT thesaurosTargets NAMESPACE thesauros:: DESTINATION ${_cfgdir})
  export(
    EXPORT thesaurosTargets
    NAMESPACE thesauros::
    FILE ${PROJECT_BINARY_DIR}/thesaurosTargets.cmake
  )

  configure_file(
    cmake/thesaurosConfig.cmake.in ${PROJECT_BINARY_DIR}/thesaurosConfig.cmake
    @ONLY
  )
  install(
    FILES ${PROJECT_BINARY_DIR}/thesaurosConfig.cmake
    DESTINATION ${_cfgdir}
  )
endif()

if(THESAUROS_BUILD_TESTS)
  # Fetch cmop for the target-option functions.
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
    "argparse/argparse"
    "argparse/reflect-enum"
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

  # Compiled against the include directory alone, mirroring the
  # `core dependency` Meson test.
  add_executable(${PROJECT_NAME}_core_dep test/core-dep.cpp)
  target_compile_features(${PROJECT_NAME}_core_dep PRIVATE cxx_std_23)
  target_include_directories(
    ${PROJECT_NAME}_core_dep
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
  cmop_add_warnings(${PROJECT_NAME}_core_dep)
  add_test(NAME ${PROJECT_NAME}.core_dep COMMAND ${PROJECT_NAME}_core_dep)

  # The structural checks over the headers themselves, mirroring the Meson
  # `check includes` test.
  find_package(Python3 COMPONENTS Interpreter)
  if(Python3_Interpreter_FOUND)
    add_test(
      NAME ${PROJECT_NAME}.check_includes
      COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/check-includes.py
    )
  endif()
endif()
