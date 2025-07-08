// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_POSTFIX_PRODUCT_INCLUSIVE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_POSTFIX_PRODUCT_INCLUSIVE_HPP

#include <cstddef>
#include <functional>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/sinks/reduce.hpp"
#include "thesauros/static-ranges/views/filter.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/static-ranges/views/transform.hpp"

namespace thes::star {
template<typename TRange>
THES_ALWAYS_INLINE inline constexpr auto postfix_product_inclusive(const TRange& range) {
  using Value = star::Value<TRange>;
  constexpr std::size_t size = thes::star::size<TRange>;

  return index_transform<size + 1>([&range](auto idx) THES_ALWAYS_INLINE {
    return static_cast<Value>(
      left_reduce(std::multiplies<>{}, Value{1})(star::only_range<iota<idx, size>>(range)));
  });
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_POSTFIX_PRODUCT_INCLUSIVE_HPP
