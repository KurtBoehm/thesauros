// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_BITSETS_HPP
#define INCLUDE_THESAUROS_FORMAT_BITSETS_HPP

#include <concepts>
#include <cstddef>

#include "thesauros/containers/bitset/dynamic.hpp"
#include "thesauros/containers/bitset/fixed.hpp"
#include "thesauros/containers/bitset/static.hpp"
#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"

namespace thes::detail {
constexpr auto write_bitset(const auto& bs, auto it) {
  for (std::size_t i = bs.size(); i > 0; --i) {
    *it++ = bs[i - 1] ? '1' : '0';
  }
  return it;
}
} // namespace thes::detail

template<std::unsigned_integral Chunk>
struct fmt::formatter<thes::DynamicBitset<Chunk>> : public thes::SimpleFormatter<> {
  auto format(const thes::DynamicBitset<Chunk>& bs, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::detail::write_bitset(bs, it); });
  }
};
template<std::size_t ChunkByteNum>
struct fmt::formatter<thes::FixedBitset<ChunkByteNum>> : public thes::SimpleFormatter<> {
  auto format(const thes::FixedBitset<ChunkByteNum>& bs, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::detail::write_bitset(bs, it); });
  }
};
template<std::size_t Size, std::size_t ChunkByteNum>
struct fmt::formatter<thes::StaticBitset<Size, ChunkByteNum>> : public thes::SimpleFormatter<> {
  auto format(const thes::StaticBitset<Size, ChunkByteNum>& bs, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::detail::write_bitset(bs, it); });
  }
};

template<std::unsigned_integral Chunk>
struct fmt::range_format_kind<thes::DynamicBitset<Chunk>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};
template<std::size_t ChunkByteNum>
struct fmt::range_format_kind<thes::FixedBitset<ChunkByteNum>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};
template<std::size_t Size, std::size_t ChunkByteNum>
struct fmt::range_format_kind<thes::StaticBitset<Size, ChunkByteNum>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};

#endif // INCLUDE_THESAUROS_FORMAT_BITSETS_HPP
