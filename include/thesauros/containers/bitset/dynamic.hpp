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

#include "thesauros/containers/array.hpp"
#include "thesauros/containers/bitset/iterator.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace thes {
template<std::unsigned_integral TChunk = std::size_t, typename TAllocator = std::allocator<TChunk>>
struct DynamicBitset {
  static_assert(std::numeric_limits<TChunk>::radix == 2);
  using Chunk = TChunk;
  static constexpr std::size_t chunk_byte_num = sizeof(Chunk);
  static constexpr std::size_t chunk_bit_num = CHAR_BIT * chunk_byte_num;

  using MutBitRef = MutableBitReference<chunk_byte_num>;
  using const_iterator = detail::BitsetIterator<DynamicBitset>;

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
    chunks_[index / chunk_bit_num] &= ~mask(index % chunk_bit_num);
  }

  [[nodiscard]] bool get(std::size_t index) const {
    assert(index < size_);
    return chunks_[index / chunk_bit_num] & mask(index % chunk_bit_num);
  }

  // Return true if set, false if already set
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

  bool operator[](std::size_t index) const {
    return get(index);
  }
  MutBitRef operator[](std::size_t i) {
    assert(i < size_);
    return MutBitRef{chunks_[i / chunk_bit_num], i % chunk_bit_num};
  }

  constexpr const_iterator begin() const {
    return const_iterator{0, *this};
  }
  constexpr const_iterator end() const {
    return const_iterator{size_, *this};
  }

private:
  static constexpr Chunk zero_chunk{0};
  static constexpr Chunk one_chunk{static_cast<Chunk>(~zero_chunk)};

  DynamicArray<Chunk, DefaultInit, DoublingGrowth, TAllocator> chunks_{};
  std::size_t size_{0};

  template<typename TCounter>
  std::size_t count(TCounter counter) const {
    if (size_ == 0) {
      return 0;
    }
    assert(div_ceil(size_, chunk_bit_num) == chunks_.size());
    const std::size_t max{chunks_.size() - 1};

    for (const std::size_t i : range(max)) {
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
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_DYNAMIC_HPP
