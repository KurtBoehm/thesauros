// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <memory>

#include "thesauros/containers/array/growth-policy.hpp"
#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/numeric-info.hpp"

namespace {
//==================================================================================================
// DoublingGrowth
//==================================================================================================

// The new size is the smallest power of two strictly above the requested lower bound, so an exact
// power of two still doubles and leaves room to grow into.
static_assert(thes::DoublingGrowth::new_allocation_size(0, 1) == 2);
static_assert(thes::DoublingGrowth::new_allocation_size(1, 2) == 4);
static_assert(thes::DoublingGrowth::new_allocation_size(2, 3) == 4);
static_assert(thes::DoublingGrowth::new_allocation_size(4, 5) == 8);
static_assert(thes::DoublingGrowth::new_allocation_size(7, 8) == 16);

/** Checks the power-of-two property over a range of lower bounds. */
THES_TEST_CASE("DoublingGrowth rounds up to a power of two", "[containers][growth-policy]") {
  for (std::size_t bound = 1; bound < 1024; ++bound) {
    const std::size_t allocated = thes::DoublingGrowth::new_allocation_size(bound - 1, bound);

    THES_CHECK(std::has_single_bit(allocated));
    THES_CHECK(allocated > bound);
    // It is the smallest such power of two, so the previous one does not exceed the bound.
    THES_CHECK(allocated / 2 <= bound);
  }
}

/** Checks that growing well past the current size still lands on a single power of two. */
THES_TEST_CASE("DoublingGrowth honours a large jump", "[containers][growth-policy]") {
  THES_CHECK(thes::DoublingGrowth::new_allocation_size(1, 100) == 128);
  THES_CHECK(thes::DoublingGrowth::new_allocation_size(1, 128) == 256);
  THES_CHECK(thes::DoublingGrowth::new_allocation_size(0, 1000) == 1024);
}

/** Checks that a request too large to round up saturates instead of shifting out of range. */
THES_TEST_CASE("DoublingGrowth saturates near the maximum", "[containers][growth-policy]") {
  using Info = thes::NumericInfo<std::size_t>;
  static constexpr std::size_t huge = std::size_t{1} << (Info::digits - 1);

  THES_CHECK(thes::DoublingGrowth::new_allocation_size(0, huge) == Info::max);
  THES_CHECK(thes::DoublingGrowth::new_allocation_size(0, Info::max) == Info::max);

  // Just below the threshold, the ordinary doubling still applies.
  THES_CHECK(thes::DoublingGrowth::new_allocation_size(0, huge - 1) == huge);
}

//==================================================================================================
// Initialization policies
//==================================================================================================

/** A class type that counts how often it has been default-constructed. */
struct Counted {
  int value{7};

  Counted() {
    ++constructions();
  }

  static int& constructions() {
    static int count{0};
    return count;
  }
};

/** Storage for `Count` objects of type `T`, deliberately filled with a non-zero byte pattern. */
template<typename T, std::size_t Count>
struct PoisonedStorage {
  PoisonedStorage() {
    std::memset(bytes.data(), 0xA5, bytes.size());
  }

  [[nodiscard]] T* begin() {
    return reinterpret_cast<T*>(bytes.data()); // NOLINT(*-reinterpret-cast)
  }
  [[nodiscard]] T* end() {
    return begin() + Count;
  }

  alignas(T) std::array<std::byte, Count * sizeof(T)> bytes{};
};

/** Checks that `ValueInit` value-initializes, which zeroes a trivial type. */
THES_TEST_CASE("ValueInit zeroes trivial storage", "[containers][initialization-policy]") {
  PoisonedStorage<int, 4> storage{};
  THES_REQUIRE(storage.begin()[0] != 0);

  thes::ValueInit::initialize(storage.begin(), storage.end());
  for (int& it : storage) {
    THES_CHECK(it == 0);
  }
  std::ranges::destroy(storage);
}

/** Checks that `DefaultInit` leaves a trivial type’s storage alone, as default-init does. */
THES_TEST_CASE("DefaultInit does not zero trivial storage", "[containers][initialization-policy]") {
  PoisonedStorage<int, 4> storage{};
  const int poison = storage.begin()[0];
  THES_REQUIRE(poison != 0);

  thes::DefaultInit::initialize(storage.begin(), storage.end());
  for (int& it : storage) {
    THES_CHECK(it == poison);
  }
  std::ranges::destroy(storage);
}

/** Checks that `DefaultInit` does run a class type’s default constructor. */
THES_TEST_CASE("DefaultInit runs default constructors", "[containers][initialization-policy]") {
  PoisonedStorage<Counted, 3> storage{};

  const int before = Counted::constructions();
  thes::DefaultInit::initialize(storage.begin(), storage.end());
  THES_CHECK(Counted::constructions() == before + 3);

  for (const Counted& it : storage) {
    THES_CHECK(it.value == 7);
  }
  std::ranges::destroy(storage);
}

/** Checks that `NoInit` touches neither the storage nor any constructor. */
THES_TEST_CASE("NoInit touches nothing", "[containers][initialization-policy]") {
  {
    PoisonedStorage<int, 4> storage{};
    const int poison = storage.begin()[0];

    thes::NoInit::initialize(storage.begin(), storage.end());
    for (int& it : storage) {
      THES_CHECK(it == poison);
    }
  }
  {
    PoisonedStorage<Counted, 3> storage{};

    const int before = Counted::constructions();
    thes::NoInit::initialize(storage.begin(), storage.end());
    THES_CHECK(Counted::constructions() == before);
  }
}

/** Checks that an empty range is a no-op for every policy. */
THES_TEST_CASE("empty ranges are a no-op", "[containers][initialization-policy]") {
  PoisonedStorage<Counted, 1> storage{};
  Counted* const begin = storage.begin();

  const int before = Counted::constructions();
  thes::ValueInit::initialize(begin, begin);
  thes::DefaultInit::initialize(begin, begin);
  thes::NoInit::initialize(begin, begin);
  THES_CHECK(Counted::constructions() == before);
}
} // namespace

THES_TEST_MAIN()
