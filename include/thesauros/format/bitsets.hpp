// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_BITSETS_HPP
#define INCLUDE_THESAUROS_FORMAT_BITSETS_HPP

#include <concepts>
#include <cstddef>

#include "thesauros/containers/bitset.hpp"
#include "thesauros/containers/bitset/dynamic.hpp"
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

template<std::unsigned_integral TChunk>
struct fmt::formatter<thes::DynamicBitset<TChunk>> : public thes::SimpleFormatter<> {
  auto format(const thes::DynamicBitset<TChunk>& bs, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::detail::write_bitset(bs, it); });
  }
};
template<std::size_t tChunkByteNum>
struct fmt::formatter<thes::FixedBitset<tChunkByteNum>> : public thes::SimpleFormatter<> {
  auto format(const thes::FixedBitset<tChunkByteNum>& bs, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::detail::write_bitset(bs, it); });
  }
};
template<std::size_t tSize, std::size_t tChunkByteNum>
struct fmt::formatter<thes::StaticBitset<tSize, tChunkByteNum>> : public thes::SimpleFormatter<> {
  auto format(const thes::StaticBitset<tSize, tChunkByteNum>& bs, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::detail::write_bitset(bs, it); });
  }
};

template<std::unsigned_integral TChunk>
struct fmt::range_format_kind<thes::DynamicBitset<TChunk>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};
template<std::size_t tChunkByteNum>
struct fmt::range_format_kind<thes::FixedBitset<tChunkByteNum>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};
template<std::size_t tSize, std::size_t tChunkByteNum>
struct fmt::range_format_kind<thes::StaticBitset<tSize, tChunkByteNum>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};

#endif // INCLUDE_THESAUROS_FORMAT_BITSETS_HPP
