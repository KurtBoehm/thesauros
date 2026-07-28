// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_BITSET_FIXED_HPP
#define INCLUDE_THESAUROS_CONTAINERS_BITSET_FIXED_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>

#include "thesauros/containers/array/fixed.hpp"
#include "thesauros/containers/bitset/iterator.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/bit.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/reduce.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/types/fixed-size-integer.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace thes {
/**
 * A bitset whose bits are packed into chunks of `ChunkByteNum` bytes, with a size that is fixed at
 * construction time. Models `std::ranges::sized_range` and `std::ranges::random_access_range`,
 * with a writable `iterator` in addition to the read-only `const_iterator`.
 */
template<std::size_t ChunkByteNum>
struct FixedBitset {
  static constexpr std::size_t chunk_byte_num = ChunkByteNum;
  static constexpr std::size_t chunk_bit_num = CHAR_BIT * chunk_byte_num;

  using Chunk = FixedUnsignedInt<chunk_byte_num>;
  static constexpr Chunk zero_chunk{0};
  static constexpr Chunk one_chunk{static_cast<Chunk>(~zero_chunk)};

  using value_type = bool;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using MutBitRef = MutableBitReference<chunk_byte_num>;
  using reference = MutBitRef;
  using const_reference = bool;
  template<bool IsConst>
  using BaseIterator = detail::BitsetIterator<FixedBitset, IsConst>;
  using iterator = BaseIterator<false>;
  using const_iterator = BaseIterator<true>;

  FixedBitset() = default;
  explicit FixedBitset(std::size_t size) : chunks_(div_ceil(size, chunk_bit_num)), size_{size} {}
  FixedBitset(std::size_t size, bool value)
      : chunks_(div_ceil(size, chunk_bit_num), value ? one_chunk : zero_chunk), size_{size} {}
  template<typename... Values>
  requires(... && std::same_as<Values, bool>)
  explicit constexpr FixedBitset(Values... values)
      : chunks_(div_ceil(sizeof...(values), chunk_bit_num)), size_{sizeof...(values)} {
    constexpr std::size_t size = sizeof...(values);
    constexpr std::size_t chunk_num = div_ceil(size, chunk_bit_num);

    std::array<bool, size> arr{values...};
    star::for_each([&](auto i) {
      constexpr auto offset = i * chunk_bit_num;
      chunks_[i] = star::left_reduce(std::bit_or<>{}, false)(
        star::index_transform<std::min(chunk_bit_num, size - offset)>(
          [&](auto j) { return Chunk{std::get<offset + j>(arr)} << j; }));
    })(star::iota<0, chunk_num>);
  }

  void set(std::size_t index) {
    assert(index < size_);
    chunks_[index / chunk_bit_num] |= mask(index % chunk_bit_num);
  }

  void unset(std::size_t index) {
    assert(index < size_);
    chunks_[index / chunk_bit_num] &= ~mask(index % chunk_bit_num);
  }

  [[nodiscard]] bool get(std::size_t index) const {
    assert(index < size_);
    return chunks_[index / chunk_bit_num] & mask(index % chunk_bit_num);
  }

  [[nodiscard]] std::size_t countr_zero() const {
    return count([](const auto x) { return thes::countr_zero(x); });
  }

  [[nodiscard]] std::size_t countr_one() const {
    return count([](const auto x) { return thes::countr_one(x); });
  }

  [[nodiscard]] std::size_t chunk_num() const {
    return chunks_.size();
  }
  [[nodiscard]] std::size_t size() const {
    return size_;
  }
  /** Whether this bitset contains no bits. */
  [[nodiscard]] bool empty() const {
    return size_ == 0;
  }

  /** The first bit. Requires `!empty()`. */
  [[nodiscard]] bool front() const {
    assert(!empty());
    return get(0);
  }
  /** The last bit. Requires `!empty()`. */
  [[nodiscard]] bool back() const {
    assert(!empty());
    return get(size_ - 1);
  }

  void fill(const bool value) {
    std::fill(chunks_.begin(), chunks_.end(), value ? one_chunk : zero_chunk);
  }

  /** Indexed access: `bool` on a `const` bitset, an assignable `MutBitRef` otherwise. */
  template<typename Self>
  [[nodiscard]] constexpr auto operator[](this Self& self, std::size_t index) {
    if constexpr (std::is_const_v<Self>) {
      return self.get(index);
    } else {
      assert(index < self.size_);
      return MutBitRef{self.chunks_[index / chunk_bit_num], index % chunk_bit_num};
    }
  }

  /** Yields `const_iterator` on a `const` bitset, the writable `iterator` otherwise. */
  template<typename Self>
  [[nodiscard]] constexpr auto begin(this Self& self) {
    return BaseIterator<std::is_const_v<Self>>{0, self};
  }
  template<typename Self>
  [[nodiscard]] constexpr auto end(this Self& self) {
    return BaseIterator<std::is_const_v<Self>>{self.size_, self};
  }
  [[nodiscard]] constexpr const_iterator cbegin() const {
    return begin();
  }
  [[nodiscard]] constexpr const_iterator cend() const {
    return end();
  }

private:
  static constexpr Chunk mask(std::size_t i) {
    return Chunk(Chunk{1} << i);
  }

  template<typename Counter>
  std::size_t count(Counter counter) const {
    if (size_ == 0) {
      return 0;
    }
    assert(div_ceil(size_, chunk_bit_num) == chunks_.size());
    const std::size_t max{chunks_.size() - 1};

    for (const std::size_t i : views::indices(max)) {
      const std::size_t count{counter(chunks_[i])};
      if (count != chunk_bit_num) {
        return count + i * chunk_bit_num;
      }
    }
    const std::size_t count{counter(chunks_.back())};
    return std::min(max * chunk_bit_num + count, size_);
  }

  FixedArray<Chunk> chunks_{};
  const std::size_t size_{0};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_FIXED_HPP
