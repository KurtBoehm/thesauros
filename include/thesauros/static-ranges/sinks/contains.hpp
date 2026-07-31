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
template<typename V>
struct ContainsGenerator : public ConsumerGeneratorBase {
  using Value = std::decay_t<V>;

  V value;

  explicit constexpr ContainsGenerator(V&& v) : value(std::forward<V>(v)) {}

  template<typename Range>
  constexpr bool operator()(Range&& range) const {
    return apply([&](const auto&... values) {
      return (... || [&]<typename T>(const T& v) {
        if constexpr (std::same_as<Value, std::decay_t<T>>) {
          return value == v;
        } else {
          return false;
        }
      }(values));
    })(std::forward<Range>(range));
  }
};

template<typename V>
constexpr auto contains(V&& value) {
  return ContainsGenerator<V>{std::forward<V>(value)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_CONTAINS_HPP
