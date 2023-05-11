#ifndef INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BITSET_HPP
#define INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BITSET_HPP

#include <atomic>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <limits>
#include <ostream>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace thes {
template<typename TChunk = std::size_t>
struct DynamicBitset {
  static_assert(std::numeric_limits<TChunk>::radix == 2 && !std::numeric_limits<TChunk>::is_signed);
  using Chunk = TChunk;
  static constexpr std::size_t chunk_size{std::numeric_limits<Chunk>::digits};

  DynamicBitset() = default;
  explicit DynamicBitset(std::size_t size) : chunks_(div_ceil(size, chunk_size)), size_{size} {}
  DynamicBitset(std::size_t size, bool value)
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

  // Return true if set, false if already set
  [[nodiscard]] bool set_if_unset(std::size_t index) {
    assert(index < size_);
    const auto index_mask = mask(index % chunk_size);

    std::atomic_ref atomic_chunk{chunks_[index / chunk_size]};
    Chunk expected = atomic_chunk;
    do {
      if (expected & index_mask) {
        return false;
      }
    } while (!atomic_chunk.compare_exchange_weak(expected, expected | index_mask));
    return true;
  }

  void push_back(bool value) {
    if (size_ == chunks_.size() * chunk_size) {
      chunks_.push_back(Chunk{value});
    } else if (value) {
      chunks_.back() |= mask(size_ % chunk_size);
    } else {
      chunks_.back() &= ~mask(size_ % chunk_size);
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
    chunks_.resize(div_ceil(size, chunk_size));
    size_ = size;
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

  friend std::ostream& operator<<(std::ostream& stream, const DynamicBitset& bitset) {
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
  static constexpr Chunk zero_chunk{0};
  static constexpr Chunk one_chunk{static_cast<Chunk>(~zero_chunk)};

  DynamicArrayDefault<Chunk> chunks_{};
  std::size_t size_{0};

  template<typename TCounter>
  std::size_t count(TCounter counter) const {
    if (size_ == 0) {
      return 0;
    }
    assert(div_ceil(size_, chunk_size) == chunks_.size());
    const std::size_t max{chunks_.size() - 1};

    for (const std::size_t i : range(max)) {
      const std::size_t count{counter(chunks_[i])};
      if (count != chunk_size) {
        return count + i * chunk_size;
      }
    }
    const std::size_t count{counter(chunks_.back())};
    return std::min(max * chunk_size + count, size_);
  }

  static constexpr Chunk mask(std::size_t i) {
    return Chunk{1} << i;
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BITSET_HPP
