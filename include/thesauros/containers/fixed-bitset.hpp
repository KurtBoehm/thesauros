#ifndef INCLUDE_THESAUROS_CONTAINERS_FIXED_BITSET_HPP
#define INCLUDE_THESAUROS_CONTAINERS_FIXED_BITSET_HPP

#include <bitset>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <ostream>

#include "thesauros/containers/array/fixed.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/bit.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/fixed-size-integer.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"
#include "thesauros/utility/static-ranges/ranges/filter.hpp"
#include "thesauros/utility/static-ranges/ranges/transform.hpp"
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
    star::iota<0, chunk_num> | star::for_each([&](auto i) {
      constexpr auto offset = i * chunk_bit_num;
      chunks_[i] = star::index_transform<std::min(chunk_bit_num, size - offset)>(
                     [&](auto j) { return Chunk{std::get<offset + j>(arr)} << j; }) |
                   star::left_reduce(std::bit_or<>{}, false);
    });
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

  friend std::ostream& operator<<(std::ostream& stream, const FixedBitset& bitset) {
    assert(div_ceil(bitset.size_, chunk_bit_num) == bitset.chunks_.size());

    if (bitset.size_ == 0) {
      return stream;
    }

    const std::size_t num{bitset.size_ / chunk_bit_num};
    const std::size_t remainder{bitset.size_ % chunk_bit_num};
    if (remainder != 0) {
      const Chunk& last{bitset.chunks_.back()};
      const std::size_t max{remainder - 1};
      for (const std::size_t i : range(remainder)) {
        stream << bool(last & mask(max - i));
      }
    }
    if (num > 0) {
      const std::size_t max{num - 1};
      for (const std::size_t i : range(num)) {
        stream << std::bitset<chunk_bit_num>(bitset.chunks_[max - i]);
      }
    }
    return stream;
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

  FixedArrayDefault<Chunk> chunks_{};
  const std::size_t size_{0};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FIXED_BITSET_HPP
