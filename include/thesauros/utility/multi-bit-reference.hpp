// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP
#define INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP

#include <atomic>
#include <cstddef>

#include "thesauros/types/fixed-size-integer.hpp"

namespace thes {
/**
 * A proxy reference to a single bit within a `Chunk`. All mutating operations are `const`, since
 * they write through the referenced `Chunk`, not to the proxy object itself; this is also what
 * `std::indirectly_writable` requires for `MutableBitReference` to model a proper writable range
 * reference, analogous to `std::vector<bool>::reference`.
 */
template<std::size_t ChunkSize>
struct MutableBitReference {
  static constexpr std::size_t chunk_size = ChunkSize;
  using Chunk = FixedUnsignedInt<chunk_size>;

  constexpr MutableBitReference(Chunk& chunk, std::size_t index) : chunk_{&chunk}, index_{index} {}

  constexpr operator bool() const { // NOLINT
    return *chunk_ & mask();
  }

  constexpr const MutableBitReference& operator=(const bool value) const { // NOLINT
    if (value) {
      *chunk_ |= mask();
    } else {
      *chunk_ &= ~mask();
    }
    return *this;
  }
  constexpr const MutableBitReference& store(const bool value, std::memory_order order) const {
    if (value) {
      std::atomic_ref{*chunk_}.fetch_or(mask(), order);
    } else {
      std::atomic_ref{*chunk_}.fetch_and(Chunk(~mask()), order);
    }
    return *this;
  }

  constexpr const MutableBitReference& operator|=(const bool value) const {
    if (value) {
      *chunk_ |= mask();
    }
    return *this;
  }
  constexpr const MutableBitReference& operator&=(const bool value) const {
    if (!value) {
      *chunk_ &= ~mask();
    }
    return *this;
  }

private:
  constexpr Chunk mask() const {
    return Chunk(Chunk{1} << index_);
  }

  Chunk* chunk_;
  const std::size_t index_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_MULTI_BIT_REFERENCE_HPP
