#ifndef INCLUDE_THESAUROS_CONTAINERS_BITSET_FIXED_HPP
#define INCLUDE_THESAUROS_CONTAINERS_BITSET_FIXED_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <functional>

#include "thesauros/containers/array/fixed.hpp"
#include "thesauros/containers/bitset/iterator.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/bit.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/fixed-size-integer.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"
#include "thesauros/utility/static-ranges/ranges/transform.hpp"
#include "thesauros/utility/static-ranges/sinks/for-each.hpp"
#include "thesauros/utility/static-ranges/sinks/reduce.hpp"

namespace thes {
template<std::size_t tChunkByteNum>
struct FixedBitset {
  static constexpr std::size_t chunk_byte_num = tChunkByteNum;
  static constexpr std::size_t chunk_bit_num = CHAR_BIT * chunk_byte_num;

  using Chunk = FixedUnsignedInt<chunk_byte_num>;
  static constexpr Chunk zero_chunk{0};
  static constexpr Chunk one_chunk{static_cast<Chunk>(~zero_chunk)};

  using MutBitRef = MutableBitReference<chunk_byte_num>;
  using const_iterator = detail::BitsetIterator<FixedBitset>;

  FixedBitset() = default;
  explicit FixedBitset(std::size_t size) : chunks_(div_ceil(size, chunk_bit_num)), size_{size} {}
  FixedBitset(std::size_t size, bool value)
      : chunks_(div_ceil(size, chunk_bit_num), value ? one_chunk : zero_chunk), size_{size} {}
  template<typename... T>
  requires(... && std::same_as<T, bool>)
  explicit constexpr FixedBitset(T... values)
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
    return Chunk{1} << i;
  }

  FixedArray<Chunk> chunks_{};
  const std::size_t size_{0};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_FIXED_HPP
