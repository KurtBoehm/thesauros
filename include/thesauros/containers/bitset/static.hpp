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
#include <type_traits>
#include <utility>

#include "thesauros/containers/bitset/iterator.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/views/constant.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/types/fixed-size-integer.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace thes {
/**
 * A fixed-size bitset which, unlike `std::bitset`, is also a proper `std::ranges::range`: it
 * exposes both a read-only `const_iterator` and a writable `iterator` whose reference type is
 * `MutBitRef`, so bits can be assigned to through iteration, not only through `operator[]`.
 */
template<std::size_t Size, std::size_t ChunkByteNum = sizeof(std::size_t)>
struct StaticBitset {
  using value_type = bool;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  static constexpr std::size_t static_size = Size;
  static constexpr std::size_t chunk_byte_num = ChunkByteNum;
  static constexpr std::size_t chunk_bit_num = CHAR_BIT * chunk_byte_num;
  static constexpr std::size_t static_chunk_num = div_ceil(static_size, chunk_bit_num);

  using Chunk = FixedUnsignedInt<chunk_byte_num>;
  static constexpr Chunk zero_chunk = 0;
  static constexpr Chunk one_chunk = static_cast<Chunk>(~zero_chunk);

  using MutBitRef = MutableBitReference<chunk_byte_num>;
  using reference = MutBitRef;
  using const_reference = bool;
  template<bool IsConst>
  using BaseIterator = detail::BitsetIterator<StaticBitset, IsConst>;
  using iterator = BaseIterator<false>;
  using const_iterator = BaseIterator<true>;

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

  /** Indexed access: `bool` on a `const` bitset, an assignable `MutBitRef` otherwise. */
  template<typename Self>
  [[nodiscard]] constexpr auto operator[](this Self& self, std::size_t index) {
    if constexpr (std::is_const_v<Self>) {
      return self.get(index);
    } else {
      assert(index < static_size);
      if constexpr (static_chunk_num == 1) {
        return MutBitRef{std::get<0>(self.chunks_), index};
      } else {
        return MutBitRef{self.chunks_[index / chunk_bit_num], index % chunk_bit_num};
      }
    }
  }

  /** Yields `const_iterator` on a `const` bitset, the writable `iterator` otherwise. */
  template<typename Self>
  [[nodiscard]] constexpr auto begin(this Self& self) {
    return BaseIterator<std::is_const_v<Self>>{0, self};
  }
  template<typename Self>
  [[nodiscard]] constexpr auto end(this Self& self) {
    return BaseIterator<std::is_const_v<Self>>{static_size, self};
  }
  [[nodiscard]] constexpr const_iterator cbegin() const {
    return begin();
  }
  [[nodiscard]] constexpr const_iterator cend() const {
    return end();
  }

private:
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
    for (const std::size_t i : views::indices(full_chunk_num)) {
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
    return Chunk(Chunk{1} << i);
  }

  std::array<Chunk, static_chunk_num> chunks_;
};

template<typename... TArgs>
explicit StaticBitset(TArgs&&... args) -> StaticBitset<sizeof...(TArgs)>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_STATIC_HPP
