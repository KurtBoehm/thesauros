#ifndef INCLUDE_THESAUROS_CONTAINERS_FIXED_BITSET_HPP
#define INCLUDE_THESAUROS_CONTAINERS_FIXED_BITSET_HPP

#include <bitset>
#include <cassert>
#include <cstddef>
#include <ostream>

#include "thesauros/containers/array/fixed.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/fixed-size-integer.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace thes {
template<std::size_t tChunkSize>
struct FixedBitset {
  static constexpr std::size_t chunk_size{tChunkSize};
  using Chunk = FixedUnsignedInt<chunk_size>;
  static constexpr Chunk zero_chunk{0};
  static constexpr Chunk one_chunk{static_cast<Chunk>(~zero_chunk)};

  explicit FixedBitset(std::size_t size) : chunks_(div_ceil(size, chunk_size)), size_{size} {}
  FixedBitset(std::size_t size, bool value)
      : chunks_(div_ceil(size, chunk_size), value ? one_chunk : zero_chunk), size_{size} {}

  void set(std::size_t index) {
    assert(index < size_);
    chunks_[index / chunk_size] |= mask(index % chunk_size);
  }

  void unset(std::size_t index) {
    assert(index < size_);
    chunks_[index / chunk_size] &= ~mask(index % chunk_size);
  }

  [[nodiscard]] bool get(std::size_t index) const {
    assert(index < size_);
    return chunks_[index / chunk_size] & mask(index % chunk_size);
  }

  [[nodiscard]] int countr_zero() const {
    return count([](const auto x) { return std::countr_zero(x); });
  }

  [[nodiscard]] int countr_one() const {
    return count([](const auto x) { return std::countr_one(x); });
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
  MutableBitReference<chunk_size> operator[](std::size_t i) {
    assert(i < size_);
    return MutableBitReference<chunk_size>{chunks_[i / chunk_size], i % chunk_size};
  }

  friend std::ostream& operator<<(std::ostream& stream, const FixedBitset& bitset) {
    assert(div_ceil(bitset.size_, chunk_size) == bitset.chunks_.size());

    if (bitset.size_ == 0) {
      return stream;
    }

    const std::size_t num{bitset.size_ / chunk_size};
    const std::size_t remainder{bitset.size_ % chunk_size};
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
        stream << std::bitset<chunk_size>(bitset.chunks_[max - i]);
      }
    }
    return stream;
  }

private:
  FixedArrayDefault<Chunk> chunks_;
  const std::size_t size_;

  template<typename TCounter>
  int count(TCounter counter) const {
    if (size_ == 0) {
      return 0;
    }
    assert(div_ceil(size_, chunk_size) == chunks_.size());
    const std::size_t max{chunks_.size() - 1};

    for (const std::size_t i : range(max)) {
      const int count{counter(chunks_[i])};
      if (count != chunk_size) {
        return count + i * chunk_size;
      }
    }
    const int count{counter(chunks_.back())};
    return std::min(max * chunk_size + count, size_);
  }

  static constexpr Chunk mask(std::size_t i) {
    return Chunk{1} << i;
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FIXED_BITSET_HPP
