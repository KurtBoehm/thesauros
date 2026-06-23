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

# pcg-cpp does not provide CMake build files
FetchContent_Declare(
  pcg_cpp
  GIT_REPOSITORY https://github.com/imneme/pcg-cpp.git
  GIT_TAG 428802d1a5634f96bcd0705fab379ff0113bcf13
)
FetchContent_MakeAvailable(pcg_cpp)

FetchContent_Declare(
  unordered_dense
  URL
    https://github.com/martinus/unordered_dense/archive/refs/tags/v4.7.0.tar.gz
  URL_HASH
    SHA256=73ce8ec4784619be4d6e54f2c4cb95d4d8dbc1fbbf24b5cf1e93b157bfa1043d
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(unordered_dense)

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
    $<INSTALL_INTERFACE:include> ${pcg_cpp_SOURCE_DIR}/include
)
target_link_libraries(
  ${PROJECT_NAME}
  INTERFACE
    Boost::preprocessor fmt::fmt ${THESAUROS_OPENMP_TARGET}
    unordered_dense::unordered_dense
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
      fmt::fmt ${THESAUROS_OPENMP_TARGET} unordered_dense::unordered_dense
  )

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
    test_name
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
    "tempfile"
    "tessellation"
    "tiling"
    "tuples"
    "type-seq"
    "value-tag"
  )
    set(test_target ${PROJECT_NAME}_${test_name})
    add_executable(${test_target} test/${test_name}.cpp)
    target_compile_features(${test_target} PRIVATE cxx_std_23)
    target_link_libraries(${test_target} PRIVATE ${PROJECT_NAME})
    cmop_add_warnings(${test_target})
    cmop_add_info(${test_target})
    add_test(NAME ${PROJECT_NAME}.${test_name} COMMAND ${test_target})
  endforeach()
endif()
