// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_CONTAINS_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_CONTAINS_HPP

#include <concepts>
#include <type_traits>
#include <utility>

#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/sinks/apply.hpp"

namespace thes::star {
template<typename TValue>
struct ContainsGenerator : public ConsumerGeneratorBase {
  using Value = std::decay_t<TValue>;

  TValue value;

  explicit constexpr ContainsGenerator(TValue&& v) : value(std::forward<TValue>(v)) {}

  template<typename TRange>
  constexpr bool operator()(TRange&& range) const {
    return apply([&](const auto&... values) {
      return (... || [&]<typename TV>(const TV& v) {
        if constexpr (std::same_as<Value, std::decay_t<TV>>) {
          return value == v;
        } else {
          return false;
        }
      }(values));
    })(std::forward<TRange>(range));
  }
};

template<typename TValue>
inline constexpr auto contains(TValue&& value) {
  return ContainsGenerator<TValue>{std::forward<TValue>(value)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_CONTAINS_HPP
