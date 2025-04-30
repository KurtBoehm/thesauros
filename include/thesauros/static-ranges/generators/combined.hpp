// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_GENERATORS_COMBINED_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_GENERATORS_COMBINED_HPP

#include <type_traits>
#include <utility>

#include "thesauros/static-ranges/definitions/concepts.hpp"

namespace thes::star {
template<IsRangeGenerator TRangeGen1, IsPipeSink TRangeGen2>
struct CombinedGenerator {
  TRangeGen1 gen1;
  TRangeGen2 gen2;

  template<typename... TRanges>
  constexpr auto operator()(TRanges&&... ranges) const& {
    return gen2(gen1(std::forward<TRanges>(ranges)...));
  }
  template<typename... TRanges>
  constexpr auto operator()(TRanges&&... ranges) && {
    return std::forward<TRangeGen2>(gen2)(
      std::forward<TRangeGen1>(gen1)(std::forward<TRanges>(ranges)...));
  }
};

template<typename TRangeGen1, typename TRangeGen2>
struct RangeGeneratorTrait<CombinedGenerator<TRangeGen1, TRangeGen2>>
    : public std::bool_constant<IsRangeGenerator<TRangeGen2>> {};

template<typename TRangeGen1, typename TRangeGen2>
struct ConsumerGeneratorTrait<CombinedGenerator<TRangeGen1, TRangeGen2>>
    : public std::bool_constant<IsConsumerGenerator<TRangeGen2>> {};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_GENERATORS_COMBINED_HPP
