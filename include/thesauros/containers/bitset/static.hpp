// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_BITSET_STATIC_HPP
#define INCLUDE_THESAUROS_CONTAINERS_BITSET_STATIC_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <utility>

#include "thesauros/containers/bitset/iterator.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/views/constant.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/types/fixed-size-integer.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace thes {
template<std::size_t tSize, std::size_t tChunkByteNum = sizeof(std::size_t)>
struct StaticBitset {
  static constexpr std::size_t static_size = tSize;
  static constexpr std::size_t chunk_byte_num = tChunkByteNum;
  static constexpr std::size_t chunk_bit_num = CHAR_BIT * chunk_byte_num;
  static constexpr std::size_t static_chunk_num = div_ceil(static_size, chunk_bit_num);

  using Chunk = FixedUnsignedInt<chunk_byte_num>;
  static constexpr Chunk zero_chunk = 0;
  static constexpr Chunk one_chunk = static_cast<Chunk>(~zero_chunk);

  using MutBitRef = MutableBitReference<chunk_byte_num>;
  using const_iterator = detail::BitsetIterator<StaticBitset>;

  constexpr StaticBitset() = default;
  explicit constexpr StaticBitset(bool value)
      : chunks_(star::to_array(star::constant<static_chunk_num>(value ? one_chunk : zero_chunk))) {}

  template<typename... TArgs>
  requires(sizeof...(TArgs) == static_size && (... && std::same_as<TArgs, bool>))
  explicit constexpr StaticBitset(TArgs&&... args)
      : chunks_(generate(std::forward<TArgs>(args)...)) {}

  constexpr void set(std::size_t index) {
    assert(index < static_size);
    if constexpr (static_chunk_num == 1) {
      std::get<0>(chunks_) |= mask(index);
    } else {
      chunks_[index / chunk_bit_num] |= mask(index % chunk_bit_num);
    }
  }

  constexpr void unset(std::size_t index) {
    assert(index < static_size);
    if constexpr (static_chunk_num == 1) {
      std::get<0>(chunks_) &= ~mask(index);
    } else {
      chunks_[index / chunk_bit_num] &= ~mask(index % chunk_bit_num);
    }
  }

  [[nodiscard]] constexpr bool get(std::size_t index) const {
    assert(index < static_size);
    if constexpr (static_chunk_num == 1) {
      return std::get<0>(chunks_) & mask(index);
    } else {
      return chunks_[index / chunk_bit_num] & mask(index % chunk_bit_num);
    }
  }

  [[nodiscard]] constexpr auto countr_zero() const {
    return countr([](const auto x) { return std::countr_zero(x); });
  }

  [[nodiscard]] constexpr auto countr_one() const {
    return countr([](const auto x) { return std::countr_one(x); });
  }

  [[nodiscard]] constexpr std::size_t chunk_num() const {
    return static_chunk_num;
  }
  [[nodiscard]] constexpr std::size_t size() const {
    return static_size;
  }

  constexpr void fill(const bool value) {
    chunks_.fill(value ? one_chunk : zero_chunk);
  }

  constexpr bool operator[](std::size_t index) const {
    return get(index);
  }
  constexpr MutBitRef operator[](std::size_t index) {
    assert(index < static_size);
    if constexpr (static_chunk_num == 1) {
      return MutBitRef{std::get<0>(chunks_), index};
    } else {
      return MutBitRef{chunks_[index / chunk_bit_num], index % chunk_bit_num};
    }
  }

  constexpr const_iterator begin() const {
    return const_iterator{0, *this};
  }
  constexpr const_iterator end() const {
    return const_iterator{static_size, *this};
  }

private:
  std::array<Chunk, static_chunk_num> chunks_;

  template<typename... TArgs>
  static constexpr auto generate(TArgs&&... args) {
    const auto data = std::make_tuple(std::forward<TArgs>(args)...);
    return star::to_array(star::index_transform<static_chunk_num>([&](auto chunk_index) {
      Chunk chunk{0};

      if constexpr ((chunk_index + 1) * chunk_bit_num <= static_size) {
        // complete chunk
        star::for_each([&](auto bit_index) {
          constexpr std::size_t index = chunk_bit_num * chunk_index + bit_index;
          chunk += Chunk{std::get<index>(data)} << bit_index;
        })(star::iota<0, chunk_bit_num>);
      } else {
        // incomplete chunk
        constexpr std::size_t full_chunk_num = static_size / chunk_bit_num;
        constexpr std::size_t remainder = static_size % chunk_bit_num;

        star::for_each([&chunk, &data](auto bit_index) {
          constexpr std::size_t index = chunk_bit_num * full_chunk_num + bit_index;
          chunk += Chunk{std::get<index>(data)} << bit_index;
        })(star::iota<0, remainder>);
      }

      return chunk;
    }));
  }

  constexpr auto countr(auto counter) const {
    if constexpr (static_chunk_num == 0) {
      return 0;
    }
    if constexpr (static_chunk_num == 1) {
      const auto count = static_cast<std::size_t>(counter(std::get<0>(chunks_)));
      return std::min(count, static_size);
    }

    constexpr std::size_t full_chunk_num = static_size / chunk_bit_num;
    for (const std::size_t i : range(full_chunk_num)) {
      const auto count = static_cast<std::size_t>(counter(chunks_[i]));
      if (count != chunk_bit_num) {
        return count + i * chunk_bit_num;
      }
    }
    if constexpr (full_chunk_num != static_chunk_num) {
      const auto count = static_cast<std::size_t>(counter(chunks_.back()));
      return std::min(full_chunk_num * chunk_bit_num + count, static_size);
    } else {
      return static_size;
    }
  }

  static constexpr Chunk mask(std::size_t i) {
    return Chunk{1} << i;
  }
};

template<typename... TArgs>
explicit StaticBitset(TArgs&&... args) -> StaticBitset<sizeof...(TArgs)>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_STATIC_HPP
