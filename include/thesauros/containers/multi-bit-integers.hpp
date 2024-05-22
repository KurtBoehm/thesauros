#ifndef INCLUDE_THESAUROS_CONTAINERS_MULTI_BIT_INTEGERS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_MULTI_BIT_INTEGERS_HPP

#include <atomic>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/array/growth-policy.hpp"
#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/math/arithmetic.hpp"

namespace thes {
template<std::unsigned_integral TChunk, std::size_t tBitNum,
         template<typename> typename TAllocator = std::allocator>
requires(std::has_single_bit(tBitNum))
struct MultiBitIntegers {
  using Chunk = TChunk;
  using Allocator = TAllocator<Chunk>;
  using Limits = std::numeric_limits<Chunk>;
  static constexpr std::size_t per_chunk = Limits::digits / tBitNum;
  static constexpr Chunk mask = Limits::max() >> (Limits::digits - tBitNum);

  struct SetProxy {
    Chunk& chunk;
    std::size_t offset;

    constexpr SetProxy& operator=(Chunk value) {
      assert(value == (value & mask));
      chunk = (chunk & ~(mask << offset)) | (value << offset);
      return *this;
    }

    void store(Chunk value, std::memory_order mem_order) {
      std::atomic_ref ref{chunk};
      for (Chunk c = ref;
           !ref.compare_exchange_weak(c, update_chunk(c, offset, value), mem_order);) {
      }
    }

    constexpr void set_bit(Chunk index, bool value) {
      chunk = thes::set_bit<Chunk>(chunk, index + offset, value);
    }
    constexpr void set_bit(Chunk index, bool value, std::memory_order mem_order) {
      std::atomic_ref ref{chunk};
      for (Chunk c = ref; !ref.compare_exchange_weak(
             c, thes::set_bit<Chunk>(c, index + offset, value), mem_order);) {
      }
    }

    constexpr bool get_bit(Chunk index) {
      return thes::get_bit<Chunk>(chunk, index + offset);
    }

    [[nodiscard]] constexpr operator Chunk() const {
      return (chunk >> offset) & mask;
    }

  private:
    static constexpr Chunk update_chunk(Chunk chunk, std::size_t offset, Chunk value) {
      return static_cast<Chunk>((chunk & ~(mask << offset)) | (value << offset));
    }
  };

  explicit constexpr MultiBitIntegers(std::size_t size)
      : data_(div_ceil(size, per_chunk)), size_(size) {}

  explicit constexpr MultiBitIntegers(std::size_t size, Chunk value)
      : data_(div_ceil(size, per_chunk)), size_(size) {
    const auto fill = [value]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/) {
      return (... | (value << (tBitNum * tIdxs)));
    }(std::make_index_sequence<per_chunk>{});
    std::fill(data_.begin(), data_.end(), fill);
  }

  [[nodiscard]] constexpr Chunk operator[](std::size_t index) const {
    assert(index < size_);
    Chunk out = data_[index / per_chunk];
    const auto offset = tBitNum * (index % per_chunk);
    return (out >> offset) & mask;
  }

  [[nodiscard]] constexpr SetProxy operator[](std::size_t index) {
    assert(index < size_);
    return {data_[index / per_chunk], static_cast<Chunk>(tBitNum * (index % per_chunk))};
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
