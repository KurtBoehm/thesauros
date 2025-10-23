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
template<bool tConst, typename T>
using MaybeConst = std::conditional_t<tConst, const T, T>;

// C++23 26.5.2
template<typename TR>
concept simple_view =
  std::ranges::view<TR> && std::ranges::range<const TR> &&
  std::same_as<std::ranges::iterator_t<TR>, std::ranges::iterator_t<const TR>> &&
  std::same_as<std::ranges::sentinel_t<TR>, std::ranges::sentinel_t<const TR>>;

// C++23 26.7.5
template<typename TF, typename TTuple>
constexpr auto tuple_transform(TF&& f, TTuple&& t) {
  return std::apply(
    [&]<typename... Ts>(Ts&&... elements) {
      return std::tuple<std::invoke_result_t<TF&, Ts>...>(
        std::invoke(f, std::forward<Ts>(elements))...);
    },
    std::forward<TTuple>(t));
}

// C++23 26.7.24.3
template<bool tConst, typename... TViews>
concept all_random_access = (std::ranges::random_access_range<MaybeConst<tConst, TViews>> && ...);

// C++23 26.7.32.2
template<bool tConst, typename TFirst, typename... TVs>
concept cartesian_product_is_random_access =
  (std::ranges::random_access_range<MaybeConst<tConst, TFirst>> && ... &&
   (std::ranges::random_access_range<MaybeConst<tConst, TVs>> &&
    std::ranges::sized_range<MaybeConst<tConst, TVs>>));
template<typename TR>
concept cartesian_product_common_arg =
  std::ranges::common_range<TR> ||
  (std::ranges::sized_range<TR> && std::ranges::random_access_range<TR>);
template<bool tConst, typename TFirst, typename... TVs>
concept cartesian_product_is_bidirectional =
  (std::ranges::bidirectional_range<MaybeConst<tConst, TFirst>> && ... &&
   (std::ranges::bidirectional_range<MaybeConst<tConst, TVs>> &&
    cartesian_product_common_arg<MaybeConst<tConst, TVs>>));
template<typename TFirst, typename... TVs>
concept cartesian_product_is_common = cartesian_product_common_arg<TFirst>;
template<typename... TVs>
concept cartesian_product_is_sized = (std::ranges::sized_range<TVs> && ...);
template<bool tConst, template<typename> typename TFirstSent, typename TFirst, typename... TVs>
concept cartesian_is_sized_sentinel =
  (std::sized_sentinel_for<TFirstSent<MaybeConst<tConst, TFirst>>,
                           std::ranges::iterator_t<MaybeConst<tConst, TFirst>>> &&
   ... &&
   (std::ranges::sized_range<MaybeConst<tConst, TVs>> &&
    std::sized_sentinel_for<std::ranges::iterator_t<MaybeConst<tConst, TVs>>,
                            std::ranges::iterator_t<MaybeConst<tConst, TVs>>>));
template<cartesian_product_common_arg TR>
constexpr auto cartesian_common_arg_end(TR& r) {
  if constexpr (std::ranges::common_range<TR>) {
    return std::ranges::end(r);
  } else {
    return std::ranges::begin(r) + std::ranges::distance(r);
  }
}
} // namespace thes::ranges::exposition

#endif // INCLUDE_THESAUROS_RANGES_EXPOSITION_HPP
