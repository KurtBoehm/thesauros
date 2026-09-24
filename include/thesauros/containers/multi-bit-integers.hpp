// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_MULTI_BIT_INTEGERS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_MULTI_BIT_INTEGERS_HPP

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>

#include "thesauros/concepts/numeric.hpp"
#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/array/growth-policy.hpp"
#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/safe-integer.hpp"
#include "thesauros/static-ranges/definitions/static-apply.hpp"

namespace thes {
template<std::unsigned_integral C, std::size_t BitN, typename A = std::allocator<C>>
requires(PowerOfTwo<BitN>)
struct MultiBitIntegers {
  using Chunk = C;
  using Allocator = A;
  using Limits = std::numeric_limits<Chunk>;
  static constexpr std::size_t per_chunk = Limits::digits / BitN;
  static constexpr Chunk mask = Limits::max() >> (Limits::digits - BitN);

  struct SetProxy {
    Chunk& chunk;
    std::size_t offset;

    constexpr SetProxy& operator=(Chunk value) {
      assert(value == (value & mask));
      chunk = update_chunk(chunk, offset, value);
      return *this;
    }

    void store(Chunk value, std::memory_order mem_order) {
      const std::atomic_ref ref{chunk};
      for (Chunk c = ref;
           !ref.compare_exchange_weak(c, update_chunk(c, offset, value), mem_order);) {
      }
    }

    constexpr void set_bit(Chunk index, bool value) {
      chunk = thes::set_bit<Chunk>(chunk, index + offset, value);
    }
    constexpr void set_bit(Chunk index, bool value, std::memory_order mem_order) {
      const std::atomic_ref ref{chunk};
      const auto bmask = SafeInt<Chunk>{1} << (index + offset);
      if (value) {
        ref.fetch_or(bmask.unsafe(), mem_order);
      } else {
        ref.fetch_and((~bmask).unsafe(), mem_order);
      }
    }

    constexpr bool get_bit(Chunk index) {
      return thes::get_bit<Chunk>(chunk, index + offset);
    }

    [[nodiscard]] constexpr operator Chunk() const { // NOLINT(*-explicit-*)
      return ((SafeInt<Chunk>{chunk} >> offset) & mask).unsafe();
    }

  private:
    static constexpr Chunk update_chunk(Chunk chunk, std::size_t offset, Chunk value) {
      using S = SafeInt<Chunk>;
      return ((S{chunk} & ~(S{mask} << offset)) | (S{value} << offset)).unsafe();
    }
  };

  explicit constexpr MultiBitIntegers(std::size_t size)
      : data_(div_ceil(size, per_chunk)), size_{size} {}

  explicit constexpr MultiBitIntegers(std::size_t size, Chunk value)
      : data_(div_ceil(size, per_chunk)), size_{size} {
    const auto fill = star::static_apply<per_chunk>(
      [value]<std::size_t... I> { return (... | (SafeInt<Chunk>{value} << (BitN * I))); });
    std::fill(data_.begin(), data_.end(), fill.unsafe());
  }

  [[nodiscard]] constexpr Chunk operator[](std::size_t index) const {
    assert(index < size_);
    const SafeInt<Chunk> out{data_[index / per_chunk]};
    const auto offset = BitN * (index % per_chunk);
    return ((out >> offset) & mask).unsafe();
  }

  [[nodiscard]] constexpr SetProxy operator[](std::size_t index) {
    assert(index < size_);
    return {data_[index / per_chunk], static_cast<Chunk>(BitN * (index % per_chunk))};
  }

  /**
   * Atomically reads the value at `index`. This is non-`const` because `std::atomic_ref` can only
   * be formed over a non-`const` object, mirroring `SetProxy::store` on the writing side.
   */
  [[nodiscard]] Chunk load(std::size_t index, std::memory_order order) {
    assert(index < size_);
    const Chunk out = std::atomic_ref{data_[index / per_chunk]}.load(order);
    const auto offset = BitN * (index % per_chunk);
    return (out >> offset) & mask;
  }

  [[nodiscard]] constexpr std::size_t size() const {
    return size_;
  }
  [[nodiscard]] constexpr std::size_t chunk_num() const {
    return data_.size();
  }

private:
  DynamicArray<Chunk, DefaultInit, DoublingGrowth, Allocator> data_;
  std::size_t size_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_MULTI_BIT_INTEGERS_HPP
