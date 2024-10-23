#ifndef INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP
#define INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP

#include <atomic>
#include <cstddef>

#include "thesauros/utility/fixed-size-integer.hpp"

namespace thes {
template<std::size_t tChunkSize>
struct MutableBitReference {
  static constexpr std::size_t chunk_size = tChunkSize;
  using Chunk = FixedUnsignedInt<chunk_size>;

  constexpr MutableBitReference(Chunk& chunk, std::size_t index) : chunk_{chunk}, index_{index} {}

  constexpr operator bool() const {
    return chunk_ & mask();
  }

  constexpr MutableBitReference& operator=(const bool value) {
    if (value) {
      chunk_ |= mask();
    } else {
      chunk_ &= ~mask();
    }
    return *this;
  }
  constexpr MutableBitReference& store(const bool value, std::memory_order order) {
    if (value) {
      std::atomic_ref{chunk_}.fetch_or(mask(), order);
    } else {
      std::atomic_ref{chunk_}.fetch_and(~mask(), order);
    }
    return *this;
  }

private:
  constexpr Chunk mask() const {
    return Chunk{1} << index_;
  }

  Chunk& chunk_;
  const std::size_t index_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP
