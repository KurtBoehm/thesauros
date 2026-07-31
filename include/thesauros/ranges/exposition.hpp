// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_EXPOSITION_HPP
#define INCLUDE_THESAUROS_RANGES_EXPOSITION_HPP

#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace thes::ranges::exposition {
// C++23 26.2
template<bool Const, typename T>
using MaybeConst = std::conditional_t<Const, const T, T>;

// C++23 26.5.2
template<typename R>
concept simple_view = std::ranges::view<R> && std::ranges::range<const R> &&
                      std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>> &&
                      std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

// C++23 26.7.5
template<typename F, typename Tuple>
constexpr auto tuple_transform(F&& f, Tuple&& t) {
  return std::apply(
    [&]<typename... Ts>(Ts&&... elements) {
      return std::tuple<std::invoke_result_t<F&, Ts>...>(
        std::invoke(f, std::forward<Ts>(elements))...);
    },
    std::forward<Tuple>(t));
}

// C++23 26.7.24.3
template<bool Const, typename... Views>
concept all_random_access = (std::ranges::random_access_range<MaybeConst<Const, Views>> && ...);

// C++23 26.7.32.2
template<bool Const, typename First, typename... Vs>
concept cartesian_product_is_random_access =
  (std::ranges::random_access_range<MaybeConst<Const, First>> && ... &&
   (std::ranges::random_access_range<MaybeConst<Const, Vs>> &&
    std::ranges::sized_range<MaybeConst<Const, Vs>>));
template<typename R>
concept cartesian_product_common_arg =
  std::ranges::common_range<R> ||
  (std::ranges::sized_range<R> && std::ranges::random_access_range<R>);
template<bool Const, typename First, typename... Vs>
concept cartesian_product_is_bidirectional =
  (std::ranges::bidirectional_range<MaybeConst<Const, First>> && ... &&
   (std::ranges::bidirectional_range<MaybeConst<Const, Vs>> &&
    cartesian_product_common_arg<MaybeConst<Const, Vs>>));
template<typename First, typename... Vs>
concept cartesian_product_is_common = cartesian_product_common_arg<First>;
template<typename... Vs>
concept cartesian_product_is_sized = (std::ranges::sized_range<Vs> && ...);
template<bool Const, template<typename> typename FirstSent, typename First, typename... Vs>
concept cartesian_is_sized_sentinel =
  (std::sized_sentinel_for<FirstSent<MaybeConst<Const, First>>,
                           std::ranges::iterator_t<MaybeConst<Const, First>>> &&
   ... &&
   (std::ranges::sized_range<MaybeConst<Const, Vs>> &&
    std::sized_sentinel_for<std::ranges::iterator_t<MaybeConst<Const, Vs>>,
                            std::ranges::iterator_t<MaybeConst<Const, Vs>>>));
template<cartesian_product_common_arg R>
constexpr auto cartesian_common_arg_end(R& r) {
  if constexpr (std::ranges::common_range<R>) {
    return std::ranges::end(r);
  } else {
    return std::ranges::begin(r) + std::ranges::distance(r);
  }
}
} // namespace thes::ranges::exposition

#endif // INCLUDE_THESAUROS_RANGES_EXPOSITION_HPP
