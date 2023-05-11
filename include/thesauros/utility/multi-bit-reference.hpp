#ifndef INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP
#define INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP

#include <cstddef>

#include "thesauros/utility/fixed-size-integer.hpp"

namespace thes {
template<std::size_t tChunkSize>
struct MutableBitReference {
  static constexpr std::size_t chunk_size = tChunkSize;
  using Chunk = FixedUnsignedInt<chunk_size>;

  MutableBitReference(Chunk& chunk, std::size_t index) : chunk_{chunk}, index_{index} {}

  operator bool() const {
    return chunk_ & mask();
  }

  MutableBitReference& operator=(const bool value) {
    if (value) {
      chunk_ |= mask();
    } else {
      chunk_ &= ~mask();
    }
    return *this;
  }

private:
  Chunk& chunk_;
  const std::size_t index_;

  constexpr Chunk mask() const {
    return Chunk{1} << index_;
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP
