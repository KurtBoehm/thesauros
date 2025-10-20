# This file is part of https://github.com/KurtBoehm/thesauros.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.20)
project(thesauros LANGUAGES CXX)

# Build options
option(THESAUROS_BUILD_TESTS "Build tests" OFF)

# Ensure we build {fmt} as a static library and don’t pull in extra stuff
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)

include(FetchContent)

# Fetch cmop to add functions that add target options
FetchContent_Declare(
  cmop
  GIT_REPOSITORY https://github.com/KurtBoehm/cmop.git
  GIT_TAG main
)
FetchContent_MakeAvailable(cmop)

find_package(OpenMP COMPONENTS CXX)

FetchContent_Declare(
  boost_preprocessor
  URL
    https://github.com/boostorg/preprocessor/archive/refs/tags/boost-1.89.0.tar.gz
  URL_HASH
    SHA256=53de1b2e64706b73903d9f3957525fa8fa620445ec0d596aa6fb8335af6f45c0
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(boost_preprocessor)

FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/12.0.0.tar.gz
  URL_HASH
    SHA256=aa3e8fbb6a0066c03454434add1f1fc23299e85758ceec0d7d2d974431481e40
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  unordered_dense
  URL
    https://github.com/martinus/unordered_dense/archive/refs/tags/v4.7.0.tar.gz
  URL_HASH
    SHA256=73ce8ec4784619be4d6e54f2c4cb95d4d8dbc1fbbf24b5cf1e93b157bfa1043d
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(unordered_dense)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(
  ${PROJECT_NAME}
  INTERFACE
    Boost::preprocessor
    fmt::fmt
    OpenMP::OpenMP_CXX
    unordered_dense::unordered_dense
)

if(THESAUROS_BUILD_TESTS)
  # pcg-cpp does not provide CMake build files
  FetchContent_Declare(
    pcg_cpp
    GIT_REPOSITORY https://github.com/imneme/pcg-cpp.git
    GIT_TAG 428802d1a5634f96bcd0705fab379ff0113bcf13
  )
  FetchContent_MakeAvailable(pcg_cpp)

  include(CTest)
  foreach(
    TEST_NAME
    IN
    ITEMS
      "arrays"
      "chunked-dynamic-array"
      "concepts"
      "divisor"
      "dynamic-bitset"
      "empty"
      "execution"
      "factorization"
      "fancy-visit"
      "fixed-bitset"
      "flat"
      "format"
      "index-position-wrapper"
      "index-segmenter"
      "json"
      "macropolis"
      "math"
      "multi-byte-integers"
      "primitives"
      "quantity"
      "randomize-range"
      "ranges"
      "static-bitset"
      "static-map"
      "static-ranges"
      "static-string"
      "std-extensions"
      "swap-or-equal"
      "tessellation"
      "tiling"
      "tuples"
      "type-seq"
      "value-tag"
  )
    add_executable(${PROJECT_NAME}_${TEST_NAME} test/${TEST_NAME}.cpp)
    target_compile_features(${PROJECT_NAME}_${TEST_NAME} PRIVATE cxx_std_23)
    target_link_libraries(${PROJECT_NAME}_${TEST_NAME} PRIVATE ${PROJECT_NAME})
    target_include_directories(
      ${PROJECT_NAME}_${TEST_NAME}
      PRIVATE ${pcg_cpp_SOURCE_DIR}/include
    )
    cmop_add_warnings(${PROJECT_NAME}_${TEST_NAME})
    cmop_add_info(${PROJECT_NAME}_${TEST_NAME})
    add_test(
      NAME ${PROJECT_NAME}.${TEST_NAME}
      COMMAND ${PROJECT_NAME}_${TEST_NAME}
    )
  endforeach()
endif()
