// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "get-at.hpp"
#include "size.hpp"
#include "type-traits.hpp"

namespace thes::star {
template<typename Range, std::size_t I>
concept SupportsGetAt = requires(const Range& range) { get_at<I>(range); };

template<typename Range>
concept AnyStaticRange =
  requires { size<Range>; } && []<std::size_t... I>(std::index_sequence<I...>) {
    return (... && SupportsGetAt<Range, I>);
  }(std::make_index_sequence<size<Range>>{});

template<typename Range>
concept AnyTypedStaticRange = AnyStaticRange<Range> && HasValue<Range>;
template<typename Range, typename T>
concept TypedStaticRange = AnyTypedStaticRange<Range> && std::same_as<Value<Range>, T>;
template<typename Range, std::size_t Size>
concept SizedStaticRange = AnyStaticRange<Range> && size<Range> == Size;
template<typename Range, typename T, std::size_t Size>
concept TypedSizedStaticRange = TypedStaticRange<Range, T> && SizedStaticRange<Range, Size>;

struct RangeGeneratorBase {};
template<typename Gen>
inline constexpr bool is_range_generator = std::derived_from<Gen, RangeGeneratorBase>;

struct ConsumerGeneratorBase {};
template<typename Gen>
inline constexpr bool is_consumer_generator = std::derived_from<Gen, ConsumerGeneratorBase>;

template<typename Gen>
concept IsRangeGenerator = is_range_generator<std::remove_cvref_t<Gen>>;
template<typename Gen>
concept IsConsumerGenerator = is_consumer_generator<std::remove_cvref_t<Gen>>;
template<typename Gen>
concept IsPipeSink = IsRangeGenerator<Gen> || IsConsumerGenerator<Gen>;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
