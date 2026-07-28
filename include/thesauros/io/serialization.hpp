// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_IO_SERIALIZATION_HPP
#define INCLUDE_THESAUROS_IO_SERIALIZATION_HPP

#include <cstddef>
#include <span>
#include <utility>

#include "thesauros/containers/array/typed-chunk.hpp"
#include "thesauros/containers/multi-byte-integers.hpp"
#include "thesauros/containers/nested-dynamic-array.hpp"
#include "thesauros/io/file-reader.hpp"
#include "thesauros/io/file-writer.hpp"
#include "thesauros/types/type-tag.hpp"

/**
 * Binary serialization of the Thesauros containers.
 *
 * These are free functions rather than members so that `containers` need not know that files
 * exist: the dependency runs from `io` to `containers` only. Every container is written as its
 * element count followed by its raw element bytes, so a `to_file`/`from_file` pair round-trips
 * only between runs that agree on element type and endianness.
 */
namespace thes {
//--------------------------------------------------------------------------------------------------
// TypedChunk
//--------------------------------------------------------------------------------------------------

/** Writes `chunk` as its element count followed by its elements. */
template<typename V, typename S, typename Alloc>
inline void to_file(const TypedChunk<V, S, Alloc>& chunk, FileWriter& writer) {
  const S stored_size = chunk.size();
  writer.write(std::span{&stored_size, 1});
  writer.write(chunk.span());
}

/** Reads a `TypedChunk` previously written by `to_file`. */
template<typename V, typename S, typename Alloc>
inline TypedChunk<V, S, Alloc> from_file(FileReader& reader,
                                         TypeTag<TypedChunk<V, S, Alloc>> /*tag*/) {
  TypedChunk<V, S, Alloc> chunk(reader.read(type_tag<S>));
  reader.read(chunk.span());
  return chunk;
}

//--------------------------------------------------------------------------------------------------
// NestedDynamicArray
//--------------------------------------------------------------------------------------------------

/** Writes `array` as its offsets followed by its values. */
template<typename T, typename TSize, typename TAlloc>
inline void to_file(const NestedDynamicArray<T, TSize, TAlloc>& array, FileWriter& writer) {
  to_file(array.offsets(), writer);
  to_file(array.values(), writer);
}

/** Reads a `NestedDynamicArray` previously written by `to_file`. */
template<typename T, typename TSize, typename TAlloc>
inline NestedDynamicArray<T, TSize, TAlloc>
from_file(FileReader& reader, TypeTag<NestedDynamicArray<T, TSize, TAlloc>> /*tag*/) {
  using Array = NestedDynamicArray<T, TSize, TAlloc>;
  auto offsets = from_file(reader, type_tag<typename Array::SizeStorage>);
  auto values = from_file(reader, type_tag<typename Array::Storage>);
  return Array{std::move(offsets), std::move(values)};
}

//--------------------------------------------------------------------------------------------------
// Multi-byte integers
//--------------------------------------------------------------------------------------------------

/** Writes `array` as its element count followed by its packed byte content. */
template<typename D, typename ByteInt, std::size_t PaddingBytes, bool IsOptional, typename Storage>
inline void
to_file(const MultiByteIntegersBase<D, ByteInt, PaddingBytes, IsOptional, Storage>& array,
        FileWriter& writer) {
  const std::size_t stored_size = array.size();
  writer.write(std::span{&stored_size, 1});
  writer.write(array.byte_span());
}

/** Reads a `MultiByteIntegerArray` previously written by `to_file`. */
template<typename ByteInt, std::size_t PaddingBytes, bool IsOptional, typename ByteAlloc>
inline MultiByteIntegerArray<ByteInt, PaddingBytes, IsOptional, ByteAlloc>
from_file(FileReader& reader,
          TypeTag<MultiByteIntegerArray<ByteInt, PaddingBytes, IsOptional, ByteAlloc>> /*tag*/) {
  MultiByteIntegerArray<ByteInt, PaddingBytes, IsOptional, ByteAlloc> out(
    reader.read(type_tag<std::size_t>));
  reader.read(out.byte_span());
  return out;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_SERIALIZATION_HPP
