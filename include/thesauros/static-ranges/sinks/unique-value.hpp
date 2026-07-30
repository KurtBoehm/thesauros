// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP

#include <cstddef>
#include <optional>
#include <utility>

#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"

namespace thes::star {
struct HasUniqueValueGenerator : public ConsumerGeneratorBase {
  template<typename R>
  constexpr bool operator()(R&& range) const {
    constexpr std::size_t size = star::size<R>;
    if constexpr (size == 0 || !HasValue<R>) {
      return false;
    } else {
      return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return (... && (get_at<0>(range) == get_at<I + 1>(range)));
      }(std::make_index_sequence<size - 1>{});
    }
  }
};

inline constexpr HasUniqueValueGenerator has_unique_value{};

struct UniqueValueGenerator : public ConsumerGeneratorBase {
  template<HasValue R>
  constexpr std::optional<Value<R>> operator()(R&& range) const {
    if (has_unique_value(range)) {
      return get_at<0>(range);
    }
    return std::nullopt;
  }
};

inline constexpr UniqueValueGenerator unique_value{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP
