// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <climits>
#include <cstddef>
#include <initializer_list>
#include <vector>

#include "thesauros/containers.hpp"
#include "thesauros/test.hpp"
#include "thesauros/utility/byte-integer.hpp"

namespace test = thes::test;

namespace {
//==================================================================================================
// Shared test utilities
//==================================================================================================

/** `2^(8 · byte_num)` computed in a wide type, avoiding overflow for narrow `Unsigned` types. */
template<typename ByteInt>
inline constexpr thes::u64 modulus_of = thes::u64{1} << (CHAR_BIT * ByteInt::byte_num);

/** Reduces `value` modulo `modulus_of<ByteInt>` and casts it down to `ByteInt::Unsigned`. */
template<typename ByteInt>
[[nodiscard]] typename ByteInt::Unsigned wrap(thes::u64 value) {
  return static_cast<typename ByteInt::Unsigned>(value % modulus_of<ByteInt>);
}

//==================================================================================================
// Round-trip tests against `std::vector`
//==================================================================================================

/**
 * Exercises `push_back`, element assignment, `pop_back`, iteration, `insert`, swapping and
 * sub-range sorting, checking the result against an equivalent `std::vector` after every step.
 */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_round_trip(auto... values) {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;
  static constexpr thes::u64 modulus = modulus_of<ByteInt>;

  static_assert(std::ranges::random_access_range<Mbi>);

  Mbi integers{values...};
  std::vector<UInt> vec{values...};

  auto elem_assert = [&integers, &vec] { THES_ALWAYS_ASSERT(test::range_eq(integers, vec)); };
  auto push_back = [&integers, &vec](UInt v) {
    integers.push_back(v);
    vec.push_back(v);
  };
  auto assign = [&integers, &vec](std::size_t i, UInt v) {
    integers[i] = v;
    vec[i] = v;
  };
  auto pop_back = [&integers, &vec] {
    integers.pop_back();
    vec.pop_back();
  };
  auto prepend = [&integers, &vec](auto range) {
    integers.insert(integers.begin(), range.begin(), range.end());
    vec.insert(vec.begin(), range.begin(), range.end());
  };
  auto append = [&integers, &vec](auto range) {
    integers.insert(integers.end(), range.begin(), range.end());
    vec.insert(vec.end(), range.begin(), range.end());
  };

  elem_assert();

  for (auto raw : std::initializer_list<thes::u64>{1, 2, 3, modulus / 5, modulus / 3}) {
    push_back(wrap<ByteInt>(raw));
    elem_assert();
  }

  assign(1, wrap<ByteInt>(4));
  if (integers.size() > 4) {
    assign(4, wrap<ByteInt>((modulus / 2) + (modulus / 8)));
  }
  elem_assert();

  if (integers.size() >= 2) {
    pop_back();
    pop_back();
    elem_assert();
  }

  for (auto&& it : integers) {
    it = it + 1;
  }
  for (auto& v : vec) {
    ++v;
  }
  elem_assert();

  prepend(std::vector<UInt>{UInt{1}, UInt{2}, UInt{3}});
  elem_assert();

  append(std::vector<UInt>{UInt{11}, UInt{22}, UInt{33}});
  elem_assert();

  {
    using std::swap;

    swap(integers[1], integers[3]);
    THES_ALWAYS_ASSERT(integers[1] == vec[3]);
    THES_ALWAYS_ASSERT(integers[3] == vec[1]);
    swap(vec[1], vec[3]);
    elem_assert();

    const UInt v0 = wrap<ByteInt>(12);
    UInt v = v0;
    swap(integers[0], v);
    THES_ALWAYS_ASSERT(integers[0] == v0);
    THES_ALWAYS_ASSERT(v == vec[0]);
    vec[0] = v0;
    elem_assert();

    const UInt v2 = v;
    swap(v, integers[2]);
    THES_ALWAYS_ASSERT(integers[2] == v2);
    THES_ALWAYS_ASSERT(v == vec[2]);
    vec[2] = v2;
    elem_assert();
  }

  auto sub = integers.sub_range(2, 4);
  static_assert(std::ranges::random_access_range<decltype(sub)>);

  std::sort(sub.begin(), sub.end());
  std::ranges::sort(sub);
  std::sort(vec.begin() + 2, vec.begin() + 4);
  elem_assert();

  while (!integers.empty()) {
    pop_back();
    elem_assert();
  }
}

/** Runs `test_round_trip` for the empty case and for a single pre-populated element. */
void run_round_trip_suite() {
  test_round_trip<thes::ByteInteger<1>>();
  test_round_trip<thes::ByteInteger<1>>(thes::ByteInteger<1>::Unsigned{13});
  test_round_trip<thes::ByteInteger<2>>();
  test_round_trip<thes::ByteInteger<2>>(thes::ByteInteger<2>::Unsigned{13});
  test_round_trip<thes::ByteInteger<3>>();
  test_round_trip<thes::ByteInteger<3>>(thes::ByteInteger<3>::Unsigned{13});
  test_round_trip<thes::ByteInteger<4>>();
  test_round_trip<thes::ByteInteger<4>>(thes::ByteInteger<4>::Unsigned{13});
}

//==================================================================================================
// Construction and factory functions
//==================================================================================================

/** Checks default, sized and initializer-list construction. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_construction() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  static_assert(std::ranges::random_access_range<Mbi>);

  const Mbi empty{};
  THES_ALWAYS_ASSERT(empty.empty());
  THES_ALWAYS_ASSERT(empty.size() == 0);
  THES_ALWAYS_ASSERT(empty.begin() == empty.end());

  const Mbi sized(5);
  THES_ALWAYS_ASSERT(sized.size() == 5);
  THES_ALWAYS_ASSERT(!sized.empty());

  const Mbi listed{UInt{1}, UInt{2}, UInt{3}};
  THES_ALWAYS_ASSERT(listed.size() == 3);
  THES_ALWAYS_ASSERT(listed[0] == 1 && listed[1] == 2 && listed[2] == 3);
}

/** Checks `create_zero` and `create_all_set`. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_factory_functions() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;
  static constexpr thes::u64 modulus = modulus_of<ByteInt>;
  static constexpr std::size_t size = 6;

  static_assert(std::ranges::random_access_range<Mbi>);

  const Mbi zeros = Mbi::create_zero(size);
  THES_ALWAYS_ASSERT(zeros.size() == size);
  THES_ALWAYS_ASSERT(std::ranges::all_of(zeros, [](UInt v) { return v == 0; }));

  const Mbi all_set = Mbi::create_all_set(size);
  THES_ALWAYS_ASSERT(all_set.size() == size);
  static constexpr UInt max_value = static_cast<UInt>(modulus - 1);
  THES_ALWAYS_ASSERT(std::ranges::all_of(all_set, [](UInt v) { return v == max_value; }));
}

/** Checks that `set_all` sets only the requested half-open index range. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_set_all() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;
  static constexpr thes::u64 modulus = modulus_of<ByteInt>;
  const UInt max_value = static_cast<UInt>(modulus - 1);

  static_assert(std::ranges::random_access_range<Mbi>);

  Mbi mbi = Mbi::create_zero(6);
  mbi.set_all(2, 4);
  for (std::size_t i = 0; i < mbi.size(); ++i) {
    const UInt expected = (i >= 2 && i < 4) ? max_value : UInt{0};
    THES_ALWAYS_ASSERT(mbi[i] == expected);
  }
}

/** Checks that `reserve` followed by repeated `push_back` yields the expected content. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_reserve() {
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  static_assert(std::ranges::random_access_range<Mbi>);

  Mbi mbi{};
  mbi.reserve(64);
  for (std::size_t i = 0; i < 40; ++i) {
    mbi.push_back(wrap<ByteInt>(i));
  }
  THES_ALWAYS_ASSERT(mbi.size() == 40);
  for (std::size_t i = 0; i < 40; ++i) {
    THES_ALWAYS_ASSERT(mbi[i] == wrap<ByteInt>(i));
  }
}

//==================================================================================================
// Element access and iteration
//==================================================================================================

/** Checks `front`, `back` and forward/reverse, const/non-const iteration. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_element_access() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  static_assert(std::ranges::random_access_range<Mbi>);

  Mbi mbi{UInt{1}, UInt{2}, UInt{3}, UInt{4}};
  const Mbi& cmbi = mbi;

  THES_ALWAYS_ASSERT(mbi.front() == 1);
  THES_ALWAYS_ASSERT(mbi.back() == 4);
  THES_ALWAYS_ASSERT(cmbi.front() == 1);
  THES_ALWAYS_ASSERT(cmbi.back() == 4);

  const std::vector<UInt> forward{1, 2, 3, 4};
  const std::vector<UInt> backward{4, 3, 2, 1};
  THES_ALWAYS_ASSERT(std::ranges::equal(mbi, forward));
  THES_ALWAYS_ASSERT(std::ranges::equal(cmbi, forward));
  THES_ALWAYS_ASSERT(
    std::ranges::equal(mbi.rbegin(), mbi.rend(), backward.begin(), backward.end()));
  THES_ALWAYS_ASSERT(
    std::ranges::equal(cmbi.crbegin(), cmbi.crend(), backward.begin(), backward.end()));

  THES_ALWAYS_ASSERT(mbi.begin() == mbi.cbegin());
  THES_ALWAYS_ASSERT(mbi.begin() < mbi.end());
  THES_ALWAYS_ASSERT(mbi.end() - mbi.begin() == 4);
}

/** Checks direct element-to-element copy and move assignment through `IntRef`. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_int_ref_assignment() {
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  Mbi mbi{wrap<ByteInt>(1), wrap<ByteInt>(2), wrap<ByteInt>(3)};

  mbi[0] = mbi[2];
  THES_ALWAYS_ASSERT(mbi[0] == wrap<ByteInt>(3));
  THES_ALWAYS_ASSERT(mbi[2] == wrap<ByteInt>(3));

  mbi[1] = std::move(mbi[0]);
  THES_ALWAYS_ASSERT(mbi[1] == wrap<ByteInt>(3));
}

/** Checks reverse iteration and mutation for the optional variant. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_optional_reverse_iteration() {
  using UInt = ByteInt::Unsigned;
  using OptMbi = thes::OptionalMultiByteIntegers<ByteInt, PaddingBytes>;
  using Opt = OptMbi::value_type;

  OptMbi mbi = OptMbi::create_empty(3);
  mbi[0] = UInt{1};
  mbi[1] = UInt{2};
  mbi[2] = UInt{3};

  auto rit = mbi.rbegin();
  THES_ALWAYS_ASSERT(Opt{*rit}.value() == UInt{3});
  *rit = UInt{30};
  THES_ALWAYS_ASSERT(Opt{mbi[2]}.value() == UInt{30});

  ++rit;
  THES_ALWAYS_ASSERT(Opt{*rit}.value() == UInt{2});
}

//==================================================================================================
// Reverse iterators
//==================================================================================================

/** Checks that `rbegin`/`rend`/`crbegin`/`crend` visit elements in reverse order. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_reverse_iteration() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  Mbi mbi{wrap<ByteInt>(1), wrap<ByteInt>(2), wrap<ByteInt>(3), wrap<ByteInt>(4), wrap<ByteInt>(5)};
  const Mbi& cmbi = mbi;
  const std::vector<UInt> backward{wrap<ByteInt>(5), wrap<ByteInt>(4), wrap<ByteInt>(3),
                                   wrap<ByteInt>(2), wrap<ByteInt>(1)};

  THES_ALWAYS_ASSERT(
    std::ranges::equal(mbi.rbegin(), mbi.rend(), backward.begin(), backward.end()));
  THES_ALWAYS_ASSERT(
    std::ranges::equal(cmbi.rbegin(), cmbi.rend(), backward.begin(), backward.end()));
  THES_ALWAYS_ASSERT(
    std::ranges::equal(mbi.crbegin(), mbi.crend(), backward.begin(), backward.end()));
  THES_ALWAYS_ASSERT(
    std::ranges::equal(cmbi.crbegin(), cmbi.crend(), backward.begin(), backward.end()));

  const Mbi empty{};
  THES_ALWAYS_ASSERT(empty.rbegin() == empty.rend());
  THES_ALWAYS_ASSERT(empty.crbegin() == empty.crend());
}

/** Checks that writing through a mutable reverse iterator updates the underlying elements. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_reverse_iterator_mutation() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  Mbi mbi{wrap<ByteInt>(1), wrap<ByteInt>(2), wrap<ByteInt>(3), wrap<ByteInt>(4)};

  UInt value = wrap<ByteInt>(10);
  for (auto it = mbi.rbegin(); it != mbi.rend(); ++it) {
    *it = value;
  }
  // The last write (to the first element) is the one that sticks, since `value` never changes.
  THES_ALWAYS_ASSERT(test::range_eq(mbi, std::vector<UInt>{value, value, value, value}));

  // Overwrite each element with a distinct value to check per-position mutation.
  std::size_t i = 0;
  for (auto it = mbi.rbegin(); it != mbi.rend(); ++it, ++i) {
    *it = wrap<ByteInt>(20 + i);
  }
  THES_ALWAYS_ASSERT(test::range_eq(mbi, std::vector<UInt>{wrap<ByteInt>(23), wrap<ByteInt>(22),
                                                           wrap<ByteInt>(21), wrap<ByteInt>(20)}));
}

/** Checks reverse iterator arithmetic, indexing, distance and ordering. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_reverse_iterator_arithmetic() {
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  Mbi mbi{wrap<ByteInt>(10), wrap<ByteInt>(20), wrap<ByteInt>(30), wrap<ByteInt>(40),
          wrap<ByteInt>(50)};
  const Mbi& cmbi = mbi;

  auto rit = cmbi.rbegin();
  THES_ALWAYS_ASSERT(rit[0] == cmbi.back());
  THES_ALWAYS_ASSERT(rit[4] == cmbi.front());
  THES_ALWAYS_ASSERT(*(rit + 2) == cmbi[2]);
  THES_ALWAYS_ASSERT(cmbi.rend() - cmbi.rbegin() == static_cast<std::ptrdiff_t>(cmbi.size()));

  auto rit2 = rit + 2;
  THES_ALWAYS_ASSERT(rit2 - rit == 2);
  rit2 -= 2;
  THES_ALWAYS_ASSERT(rit2 == rit);
  rit2 += 3;
  THES_ALWAYS_ASSERT(rit2 == rit + 3);
  --rit2;
  THES_ALWAYS_ASSERT(rit2 == rit + 2);

  THES_ALWAYS_ASSERT(rit < rit + 1);
  THES_ALWAYS_ASSERT(rit + 1 > rit);
  THES_ALWAYS_ASSERT(rit <= rit);
  THES_ALWAYS_ASSERT(rit >= rit);
  THES_ALWAYS_ASSERT(rit != rit + 1);
}

/** Checks that a mutable reverse iterator converts to a const reverse iterator. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_reverse_iterator_conversion() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;
  using ConstRevIt = typename Mbi::const_reverse_iterator;

  Mbi mbi{UInt{1}, UInt{2}, UInt{3}};
  auto rit = mbi.rbegin();
  const ConstRevIt crit = rit;

  THES_ALWAYS_ASSERT(crit == mbi.crbegin());
  THES_ALWAYS_ASSERT(*crit == *rit);
}

/** Checks reverse iteration and mutation through a sub-range, propagating to the parent array. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_sub_range_reverse_iteration() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  Mbi mbi{wrap<ByteInt>(1), wrap<ByteInt>(2), wrap<ByteInt>(3), wrap<ByteInt>(4), wrap<ByteInt>(5)};

  auto sub = mbi.sub_range(1, 4);
  const std::vector<UInt> backward{wrap<ByteInt>(4), wrap<ByteInt>(3), wrap<ByteInt>(2)};
  THES_ALWAYS_ASSERT(
    std::ranges::equal(sub.rbegin(), sub.rend(), backward.begin(), backward.end()));

  *sub.rbegin() = wrap<ByteInt>(40);
  THES_ALWAYS_ASSERT(mbi[3] == wrap<ByteInt>(40));

  const Mbi& cmbi = mbi;
  auto csub = cmbi.sub_range(1, 4);
  const std::vector<UInt> backward2{wrap<ByteInt>(40), wrap<ByteInt>(3), wrap<ByteInt>(2)};
  THES_ALWAYS_ASSERT(
    std::ranges::equal(csub.rbegin(), csub.rend(), backward2.begin(), backward2.end()));
}

/** Checks that reverse iterators satisfy the requirements used by `std::ranges::sort`. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_reverse_iterator_with_algorithms() {
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  Mbi mbi{wrap<ByteInt>(3), wrap<ByteInt>(1), wrap<ByteInt>(4), wrap<ByteInt>(1), wrap<ByteInt>(5)};
  std::ranges::sort(mbi);
  THES_ALWAYS_ASSERT(std::ranges::is_sorted(mbi));
  THES_ALWAYS_ASSERT(std::ranges::is_sorted(mbi.rbegin(), mbi.rend(), std::ranges::greater{}));

  // Sorting through the reverse iterators should leave the forward range in descending order.
  std::ranges::sort(mbi.rbegin(), mbi.rend());
  THES_ALWAYS_ASSERT(std::ranges::is_sorted(mbi, std::ranges::greater{}));
}

//==================================================================================================
// Sub-ranges and byte spans
//==================================================================================================

/** Checks that `byte_span` reports `size() · byte_num` bytes for both const and mutable access. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_byte_span() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;
  static_assert(std::ranges::random_access_range<Mbi>);

  Mbi mbi{UInt{1}, UInt{2}, UInt{3}};
  const Mbi& cmbi = mbi;
  const std::size_t expected_bytes = mbi.size() * ByteInt::byte_num;

  THES_ALWAYS_ASSERT(mbi.byte_span().size() == expected_bytes);
  THES_ALWAYS_ASSERT(cmbi.byte_span().size() == expected_bytes);
}

/** Checks mutable `sub_range` sorting and const `full_sub_range` access. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_sub_range() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;
  static_assert(std::ranges::random_access_range<Mbi>);

  Mbi mbi{UInt{5}, UInt{3}, UInt{1}, UInt{4}, UInt{2}};

  auto sub = mbi.sub_range(1, 4);
  static_assert(std::ranges::random_access_range<decltype(sub)>);
  THES_ALWAYS_ASSERT(sub.size() == 3);
  std::sort(sub.begin(), sub.end());
  THES_ALWAYS_ASSERT(test::range_eq(mbi, std::vector<UInt>{5, 1, 3, 4, 2}));

  const Mbi& cmbi = mbi;
  auto full = cmbi.full_sub_range();
  static_assert(std::ranges::random_access_range<decltype(full)>);
  THES_ALWAYS_ASSERT(full.size() == cmbi.size());
  THES_ALWAYS_ASSERT(std::ranges::equal(full, cmbi));
}

//==================================================================================================
// Optional variant
//==================================================================================================

/** Checks `create_empty` together with assignment, `push_back` and `pop_back`. */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_optional_variant() {
  using UInt = ByteInt::Unsigned;
  using OptMbi = thes::OptionalMultiByteIntegers<ByteInt, PaddingBytes>;
  static_assert(std::ranges::random_access_range<OptMbi>);

  OptMbi mbi = OptMbi::create_empty(4);
  const OptMbi& cmbi = mbi;
  THES_ALWAYS_ASSERT(mbi.size() == 4);

  mbi[1] = UInt{7};
  mbi[3] = UInt{2};
  THES_ALWAYS_ASSERT(cmbi[1].value() == UInt{7});
  THES_ALWAYS_ASSERT(cmbi[3].value() == UInt{2});

  mbi.push_back(UInt{9});
  THES_ALWAYS_ASSERT(mbi.size() == 5);
  THES_ALWAYS_ASSERT(cmbi.back().value() == UInt{9});

  mbi.pop_back();
  THES_ALWAYS_ASSERT(mbi.size() == 4);
}

//==================================================================================================
// insert_any: direct exercise of size/pad_end combinations, no-ops, and reallocation
//==================================================================================================

/**
 * Directly exercises `insert_any` with a range of `ins_size`/`pad_end` combinations, both within
 * existing capacity and forcing reallocation, checking the resulting size and that the gap is
 * writable and previously-surrounding data is preserved.
 */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_insert_any_direct() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  // No-op: ins_size == pad_end == 0 must not change size, contents, or the returned position.
  {
    Mbi mbi{wrap<ByteInt>(1), wrap<ByteInt>(2), wrap<ByteInt>(3)};
    auto it = mbi.insert_any(mbi.begin() + 1, 0, 0);
    THES_ALWAYS_ASSERT(mbi.size() == 3);
    THES_ALWAYS_ASSERT(
      test::range_eq(mbi, std::vector<UInt>{wrap<ByteInt>(1), wrap<ByteInt>(2), wrap<ByteInt>(3)}));
    THES_ALWAYS_ASSERT(it == mbi.begin() + 1);
  }

  // Insert into a default-constructed (zero-capacity) container.
  {
    Mbi mbi{};
    auto it = mbi.insert_any(mbi.begin(), 2, 0);
    THES_ALWAYS_ASSERT(it == mbi.begin());
    const std::vector<UInt> vals{wrap<ByteInt>(9), wrap<ByteInt>(8)};
    mbi.copy_uninit(mbi.begin(), vals.begin(), vals.end());
    THES_ALWAYS_ASSERT(mbi.size() == 2);
    THES_ALWAYS_ASSERT(mbi[0] == wrap<ByteInt>(9) && mbi[1] == wrap<ByteInt>(8));
  }

  // pad_end > 0: size grows by ins_size + pad_end; the ins_size elements at the returned iterator
  // and the trailing pad_end elements must be independently writable without disturbing the
  // elements originally before or after the insertion point.
  {
    Mbi mbi{wrap<ByteInt>(1), wrap<ByteInt>(2), wrap<ByteInt>(3), wrap<ByteInt>(4)};
    auto it = mbi.insert_any(mbi.begin() + 2, 2, 3);
    THES_ALWAYS_ASSERT(mbi.size() == 4 + 2 + 3);
    const std::vector<UInt> mid{wrap<ByteInt>(50), wrap<ByteInt>(51)};
    mbi.copy_uninit(it, mid.begin(), mid.end());
    for (std::size_t i = 0; i < 3; ++i) {
      mbi[mbi.size() - 3 + i] = wrap<ByteInt>(90 + i);
    }
    const std::vector<UInt> expected{wrap<ByteInt>(1),  wrap<ByteInt>(2),  wrap<ByteInt>(50),
                                     wrap<ByteInt>(51), wrap<ByteInt>(3),  wrap<ByteInt>(4),
                                     wrap<ByteInt>(90), wrap<ByteInt>(91), wrap<ByteInt>(92)};
    THES_ALWAYS_ASSERT(test::range_eq(mbi, expected));
  }

  // Force reallocation: reserve a small amount, then insert far more than the reserved headroom.
  {
    Mbi mbi{};
    mbi.reserve(4);
    for (std::size_t i = 0; i < 4; ++i) {
      mbi.push_back(wrap<ByteInt>(i));
    }
    std::vector<UInt> big{};
    for (std::size_t i = 0; i < 100; ++i) {
      big.push_back(wrap<ByteInt>(1000 + i));
    }
    mbi.insert(mbi.begin() + 2, big.begin(), big.end());
    THES_ALWAYS_ASSERT(mbi.size() == 104);
    std::vector<UInt> expected{wrap<ByteInt>(0), wrap<ByteInt>(1)};
    expected.insert(expected.end(), big.begin(), big.end());
    expected.push_back(wrap<ByteInt>(2));
    expected.push_back(wrap<ByteInt>(3));
    THES_ALWAYS_ASSERT(test::range_eq(mbi, expected));
  }
}

//==================================================================================================
// insert: exhaustive positions against std::vector, across repeated growth
//==================================================================================================

/**
 * Inserts small chunks at a spread of positions (begin, middle, end) over several rounds, comparing
 * against `std::vector` after every insertion, so both the in-capacity and reallocating branches of
 * `insert_any` get exercised repeatedly and at varied offsets.
 */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_insert_all_positions() {
  using UInt = ByteInt::Unsigned;
  using Mbi = thes::MultiByteIntegers<ByteInt, PaddingBytes>;

  Mbi mbi{};
  std::vector<UInt> vec{};
  thes::u64 counter = 0;

  for (int round = 0; round < 6; ++round) {
    const std::size_t initial_size = vec.size();
    for (std::size_t pos = 0; pos <= initial_size; ++pos) {
      std::vector<UInt> chunk;
      for (std::size_t i = 0; i < 3; ++i) {
        chunk.push_back(wrap<ByteInt>(counter++));
      }
      mbi.insert(mbi.begin() + static_cast<std::ptrdiff_t>(pos), chunk.begin(), chunk.end());
      vec.insert(vec.begin() + static_cast<std::ptrdiff_t>(pos), chunk.begin(), chunk.end());
      THES_ALWAYS_ASSERT(mbi.size() == vec.size());
      THES_ALWAYS_ASSERT(test::range_eq(mbi, vec));
    }
  }
}

//==================================================================================================
// insert on the optional variant
//==================================================================================================

/**
 * Checks that `insert` on `OptionalMultiByteIntegers` widens the array correctly and leaves values
 * before and after the inserted range untouched.
 */
template<typename ByteInt, std::size_t PaddingBytes = 13>
void test_optional_insert() {
  using UInt = ByteInt::Unsigned;
  using OptMbi = thes::OptionalMultiByteIntegers<ByteInt, PaddingBytes>;
  using Opt = OptMbi::value_type;

  OptMbi mbi = OptMbi::create_empty(3);
  mbi[0] = UInt{1};
  mbi[1] = UInt{2};
  mbi[2] = UInt{3};

  const std::vector<UInt> extra{UInt{7}, UInt{8}};
  mbi.insert(mbi.begin() + 1, extra.begin(), extra.end());

  THES_ALWAYS_ASSERT(mbi.size() == 5);
  THES_ALWAYS_ASSERT(Opt{mbi[0]}.value() == UInt{1});
  THES_ALWAYS_ASSERT(Opt{mbi[1]}.value() == UInt{7});
  THES_ALWAYS_ASSERT(Opt{mbi[2]}.value() == UInt{8});
  THES_ALWAYS_ASSERT(Opt{mbi[3]}.value() == UInt{2});
  THES_ALWAYS_ASSERT(Opt{mbi[4]}.value() == UInt{3});
}

//==================================================================================================
// Padding-boundary regression check
//==================================================================================================

/**
 * With `PaddingBytes` set to exactly `int_bytes` (the minimum allowed), repeatedly prepends and
 * appends so the data region’s boundary crosses back and forth over the padding, catching
 * off-by-one errors in how `insert_any` computes absolute offsets into the padded byte array.
 */
template<typename ByteInt>
void test_insert_padding_boundary() {
  using UInt = ByteInt::Unsigned;
  static constexpr std::size_t padding = sizeof(typename ByteInt::Unsigned);
  using Mbi = thes::MultiByteIntegers<ByteInt, padding>;

  Mbi mbi{};
  std::vector<UInt> vec{};

  for (std::size_t i = 0; i < 20; ++i) {
    const UInt v = wrap<ByteInt>(i);
    if (i % 4 == 0) {
      mbi.insert(mbi.begin(), &v, &v + 1);
      vec.insert(vec.begin(), v);
    } else {
      mbi.push_back(v);
      vec.push_back(v);
    }
    THES_ALWAYS_ASSERT(test::range_eq(mbi, vec));
  }
}

//==================================================================================================
// Suite runner
//==================================================================================================

/** Runs every `ByteInt`-parameterized test for a single byte width. */
template<typename ByteInt>
void run_full_suite() {
  test_construction<ByteInt>();
  test_factory_functions<ByteInt>();
  test_set_all<ByteInt>();
  test_reserve<ByteInt>();
  test_element_access<ByteInt>();
  test_int_ref_assignment<ByteInt>();
  test_reverse_iteration<ByteInt>();
  test_reverse_iterator_mutation<ByteInt>();
  test_reverse_iterator_arithmetic<ByteInt>();
  test_reverse_iterator_conversion<ByteInt>();
  test_byte_span<ByteInt>();
  test_sub_range<ByteInt>();
  test_sub_range_reverse_iteration<ByteInt>();
  test_reverse_iterator_with_algorithms<ByteInt>();
  test_optional_variant<ByteInt>();
  test_optional_reverse_iteration<ByteInt>();
  test_insert_any_direct<ByteInt>();
  test_insert_all_positions<ByteInt>();
  test_optional_insert<ByteInt>();
  test_insert_padding_boundary<ByteInt>();
}
} // namespace

int main() {
  run_round_trip_suite();

  run_full_suite<thes::ByteInteger<1>>();
  run_full_suite<thes::ByteInteger<2>>();
  run_full_suite<thes::ByteInteger<3>>();
  run_full_suite<thes::ByteInteger<4>>();
}
