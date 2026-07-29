// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <type_traits>

#include "thesauros/test/test.hpp"
#include "thesauros/types/empty.hpp"
#include "thesauros/types/void-storage.hpp"

namespace {
//==================================================================================================
// Non-void types pass through unchanged
//==================================================================================================

static_assert(std::same_as<thes::VoidStorage<int>, int>);
static_assert(std::same_as<thes::VoidRvalRef<int>, int&&>);
static_assert(std::same_as<thes::VoidStorageRvalRef<int>, int&&>);
static_assert(std::same_as<thes::VoidLvalRef<int>, int&>);
static_assert(std::same_as<thes::VoidStorageLvalRef<int>, int&>);
static_assert(std::same_as<thes::VoidConstLvalRef<int>, const int&>);
static_assert(std::same_as<thes::VoidStorageConstLvalRef<int>, const int&>);
static_assert(std::same_as<thes::VoidConstPtr<int>, const int*>);
static_assert(std::same_as<thes::VoidStorageConstPtr<int>, const int*>);

//==================================================================================================
// `void` maps to `Empty` for storage, and stays `void` otherwise
//==================================================================================================

static_assert(std::same_as<thes::VoidStorage<void>, thes::Empty>);
static_assert(std::same_as<thes::VoidRvalRef<void>, void>);
static_assert(std::same_as<thes::VoidLvalRef<void>, void>);
static_assert(std::same_as<thes::VoidConstLvalRef<void>, void>);
static_assert(std::same_as<thes::VoidConstPtr<void>, void>);

// The `Storage` variants collapse to `Empty` regardless of the transformations requested, since
// there is nothing to add a reference or pointer to.
static_assert(std::same_as<thes::VoidStorageRvalRef<void>, thes::Empty>);
static_assert(std::same_as<thes::VoidStorageLvalRef<void>, thes::Empty>);
static_assert(std::same_as<thes::VoidStorageConstLvalRef<void>, thes::Empty>);
static_assert(std::same_as<thes::VoidStorageConstPtr<void>, thes::Empty>);

//==================================================================================================
// UnVoidStorage inverts the mapping
//==================================================================================================

static_assert(std::same_as<thes::UnVoidStorage<thes::Empty>, void>);
static_assert(std::same_as<thes::UnVoidStorage<const thes::Empty&>, void>);
static_assert(std::same_as<thes::UnVoidStorage<int>, int>);
static_assert(std::same_as<thes::UnVoidStorage<const int&>, const int&>);

// Round-tripping a non-void type through storage and back is the identity.
static_assert(std::same_as<thes::UnVoidStorage<thes::VoidStorage<int>>, int>);
static_assert(std::same_as<thes::UnVoidStorage<thes::VoidStorage<void>>, void>);

//==================================================================================================
// ApplyTypeTransformationsTrait
//==================================================================================================

static_assert(std::same_as<thes::ApplyTypeTransformationsTrait<int>::Type, int>);
static_assert(
  std::same_as<thes::ApplyTypeTransformationsTrait<int, std::add_const>::Type, const int>);
// Transformations apply left to right, so the pointer is added to the already-const type.
static_assert(
  std::same_as<thes::ApplyTypeTransformationsTrait<int, std::add_const, std::add_pointer>::Type,
               const int*>);
static_assert(
  std::same_as<thes::ApplyTypeTransformationsTrait<int, std::add_pointer, std::add_const>::Type,
               int* const>);

//==================================================================================================
// The accessor functions
//==================================================================================================

/** Checks that `void_storage_cref` and `void_storage_cptr` return views of an ordinary value. */
THES_TEST_CASE("the accessors expose an ordinary value", "[types][void-storage]") {
  const int value = 42;

  static_assert(std::same_as<decltype(thes::void_storage_cref(value)), const int&>);
  static_assert(std::same_as<decltype(thes::void_storage_cptr(value)), const int*>);

  THES_CHECK(thes::void_storage_cref(value) == 42);
  THES_CHECK(thes::void_storage_cptr(value) == &value);
  THES_CHECK(*thes::void_storage_cptr(value) == 42);
}

/**
 * Checks that an `Empty` argument, which stands in for `void`, yields an `Empty` again: there is
 * no reference or pointer to form, so the storage type is passed straight through.
 */
THES_TEST_CASE("Empty stands in for void", "[types][void-storage]") {
  const thes::Empty empty{};

  static_assert(std::same_as<decltype(thes::void_storage_cref(empty)), thes::Empty>);
  static_assert(std::same_as<decltype(thes::void_storage_cptr(empty)), thes::Empty>);

  THES_CHECK(thes::void_storage_cref(empty) == empty);
  THES_CHECK(thes::void_storage_cptr(empty) == empty);
}
} // namespace

THES_TEST_MAIN()
