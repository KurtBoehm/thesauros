// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_BITSET_DYNAMIC_HPP
#define INCLUDE_THESAUROS_CONTAINERS_BITSET_DYNAMIC_HPP

#include <algorithm>
#include <atomic>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/containers/bitset/iterator.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace thes {
/**
 * A bitset whose bits are packed into chunks of `C`, growing dynamically via `push_back` and
 * `resize`. Models `std::ranges::sized_range` and `std::ranges::random_access_range`, with a
 * writable `iterator` in addition to the read-only `const_iterator`.
 */
template<std::unsigned_integral C = std::size_t, typename Alloc = std::allocator<C>>
struct DynamicBitset {
  static_assert(std::numeric_limits<C>::radix == 2);
  using Chunk = C;
  static constexpr std::size_t chunk_byte_num = sizeof(Chunk);
  static constexpr std::size_t chunk_bit_num = CHAR_BIT * chunk_byte_num;

  using value_type = bool;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using MutBitRef = MutableBitReference<chunk_byte_num>;
  using reference = MutBitRef;
  using const_reference = bool;
  template<bool IsConst>
  using BaseIterator = detail::BitsetIterator<DynamicBitset, IsConst>;
  using iterator = BaseIterator<false>;
  using const_iterator = BaseIterator<true>;

  DynamicBitset() = default;
  explicit DynamicBitset(std::size_t size) : chunks_(div_ceil(size, chunk_bit_num)), size_{size} {}
  DynamicBitset(std::size_t size, bool value)
      : chunks_(div_ceil(size, chunk_bit_num), value ? one_chunk : zero_chunk), size_{size} {}

  void set(std::size_t index) {
    assert(index < size_);
    chunks_[index / chunk_bit_num] |= mask(index % chunk_bit_num);
  }

  void unset(std::size_t index) {
    assert(index < size_);
    chunks_[index / chunk_bit_num] &= static_cast<Chunk>(~mask(index % chunk_bit_num));
  }

  [[nodiscard]] bool get(std::size_t index) const {
    assert(index < size_);
    return chunks_[index / chunk_bit_num] & mask(index % chunk_bit_num);
  }

  /** Atomically sets a bit, returning whether it was previously unset. */
  [[nodiscard]] bool set_if_unset(std::size_t index) {
    assert(index < size_);
    const auto index_mask = mask(index % chunk_bit_num);

    std::atomic_ref atomic_chunk{chunks_[index / chunk_bit_num]};
    const Chunk prev = atomic_chunk.fetch_or(index_mask);
    return (prev & index_mask) == 0;
  }

  void push_back(bool value) {
    if (size_ == chunks_.size() * chunk_bit_num) {
      chunks_.push_back(Chunk{value});
    } else if (value) {
      chunks_.back() |= mask(size_ % chunk_bit_num);
    } else {
      chunks_.back() &= ~mask(size_ % chunk_bit_num);
    }
    ++size_;
  }

  void clear() {
    chunks_.clear();
    size_ = 0;
  }

  [[nodiscard]] std::size_t countr_zero() const {
    return count([](const auto x) { return static_cast<std::size_t>(std::countr_zero(x)); });
  }

  [[nodiscard]] std::size_t countr_one() const {
    return count([](const auto x) { return static_cast<std::size_t>(std::countr_one(x)); });
  }

  [[nodiscard]] std::size_t chunk_num() const {
    return chunks_.size();
  }
  [[nodiscard]] std::size_t size() const {
    return size_;
  }
  void resize(const std::size_t size) {
    chunks_.resize(div_ceil(size, chunk_bit_num));
    size_ = size;
  }

  void fill(const bool value) {
    std::fill(chunks_.begin(), chunks_.end(), value ? one_chunk : zero_chunk);
  }

  /** Indexed access: `bool` on a `const` bitset, an assignable `MutBitRef` otherwise. */
  template<typename Self>
  [[nodiscard]] auto operator[](this Self& self, std::size_t index) {
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
  static constexpr Chunk zero_chunk{0};
  static constexpr Chunk one_chunk{static_cast<Chunk>(~zero_chunk)};

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

  static constexpr Chunk mask(std::size_t i) {
    return Chunk(Chunk{1} << i);
  }

  DynamicArray<Chunk, DefaultInit, DoublingGrowth, Alloc> chunks_{};
  std::size_t size_ = 0;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_DYNAMIC_HPP
