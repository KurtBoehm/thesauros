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
template<IsRangeGenerator R1, IsPipeSink R2>
struct CombinedGenerator {
  R1 gen1;
  R2 gen2;

  template<typename... Rs>
  constexpr auto operator()(Rs&&... ranges) const& {
    return gen2(gen1(std::forward<Rs>(ranges)...));
  }
  template<typename... Rs>
  constexpr auto operator()(Rs&&... ranges) && {
    return std::forward<R2>(gen2)(std::forward<R1>(gen1)(std::forward<Rs>(ranges)...));
  }
};

template<typename R1, typename R2>
struct RangeGeneratorTrait<CombinedGenerator<R1, R2>>
    : public std::bool_constant<IsRangeGenerator<R2>> {};

template<typename R1, typename R2>
struct ConsumerGeneratorTrait<CombinedGenerator<R1, R2>>
    : public std::bool_constant<IsConsumerGenerator<R2>> {};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_GENERATORS_COMBINED_HPP
