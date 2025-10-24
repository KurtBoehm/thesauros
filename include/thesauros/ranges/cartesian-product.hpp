// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_CARTESIAN_PRODUCT_HPP
#define INCLUDE_THESAUROS_RANGES_CARTESIAN_PRODUCT_HPP

#include <ranges>

#if __cpp_lib_ranges_cartesian_product < 202207L
#include <concepts>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "thesauros/ranges/exposition.hpp"
#include "thesauros/types/signedness.hpp"
#endif

namespace thes {
#if __cpp_lib_ranges_cartesian_product >= 202207L
namespace ranges {
template<std::ranges::input_range TFirst, std::ranges::forward_range... TVs>
using CartesianProductView = std::ranges::cartesian_product_view<TFirst, TVs...>;
}
namespace views {
inline constexpr auto cartesian_product = std::views::cartesian_product;
}
#else
namespace ranges {
// C++23 26.7.32.2
template<std::ranges::input_range TFirst, std::ranges::forward_range... TVs>
requires(std::ranges::view<TFirst> && ... && std::ranges::view<TVs>)
struct CartesianProductView
    : public std::ranges::view_interface<CartesianProductView<TFirst, TVs...>> {
  template<bool tConst>
  struct Iterator {
    using iterator_category = std::input_iterator_tag;
    // C++23 26.7.32.3 §1
    using iterator_concept = decltype([] {
      if constexpr (exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>) {
        return std::random_access_iterator_tag{};
      } else if constexpr (exposition::cartesian_product_is_bidirectional<tConst, TFirst, TVs...>) {
        return std::bidirectional_iterator_tag{};
      } else if constexpr (std::ranges::forward_range<exposition::MaybeConst<tConst, TFirst>>) {
        return std::forward_iterator_tag{};
      } else {
        return std::input_iterator_tag{};
      }
    }());
    using value_type =
      std::tuple<std::ranges::range_value_t<exposition::MaybeConst<tConst, TFirst>>,
                 std::ranges::range_value_t<exposition::MaybeConst<tConst, TVs>>...>;
    using reference =
      std::tuple<std::ranges::range_reference_t<exposition::MaybeConst<tConst, TFirst>>,
                 std::ranges::range_reference_t<exposition::MaybeConst<tConst, TVs>>...>;
    // C++23 26.7.32.3 §2
    using difference_type = CartesianProductView::difference_type;

    Iterator() = default;

    // C++23 26.7.32.3 §11
    constexpr Iterator(Iterator<!tConst> i)
    requires(tConst && (std::convertible_to<std::ranges::iterator_t<TFirst>,
                                            std::ranges::iterator_t<const TFirst>> &&
                        ... &&
                        std::convertible_to<std::ranges::iterator_t<TVs>,
                                            std::ranges::iterator_t<const TVs>>))
        : parent_(std::addressof(i.parent_)), current_(std::move(i.current_)) {}

    // C++23 26.7.32.3 §12
    constexpr auto operator*() const {
      return exposition::tuple_transform([](auto& i) -> decltype(auto) { return *i; }, current_);
    }

    // C++23 26.7.32.3 §13
    constexpr Iterator& operator++() {
      next();
      return *this;
    }

    // C++23 26.7.32.3 §14
    constexpr void operator++(int) {
      ++*this;
    }

    // C++23 26.7.32.3 §15
    constexpr Iterator operator++(int)
    requires(std::ranges::forward_range<exposition::MaybeConst<tConst, TFirst>>)
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    // C++23 26.7.32.3 §16
    constexpr Iterator& operator--()
    requires(exposition::cartesian_product_is_bidirectional<tConst, TFirst, TVs...>)
    {
      prev();
      return *this;
    }

    // C++23 26.7.32.3 §17
    constexpr Iterator operator--(int)
    requires(exposition::cartesian_product_is_bidirectional<tConst, TFirst, TVs...>)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    // C++23 26.7.32.3 §18-22
    constexpr Iterator& operator+=(difference_type x)
    requires(exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>)
    {
      // Outsourced to a fancy, template-recursive implementation
      advance(x);
      return *this;
    }

    // C++23 26.7.32.3 §23
    constexpr Iterator& operator-=(difference_type x)
    requires(exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>)
    {
      return *this += -x;
    }

    // C++23 26.7.32.3 §24
    constexpr reference operator[](difference_type n) const
    requires(exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>)
    {
      return *((*this) + n);
    }

    // C++23 26.7.32.3 §25
    friend constexpr bool operator==(const Iterator& x, const Iterator& y)
    requires(
      std::equality_comparable<std::ranges::iterator_t<exposition::MaybeConst<tConst, TFirst>>>)
    {
      return x.current_ == y.current_;
    }

    // C++23 26.7.32.3 §26
    friend constexpr bool operator==(const Iterator& x, std::default_sentinel_t) {
      return [&]<std::size_t... tIs>(std::index_sequence<tIs...>) {
        return ((std::get<tIs>(x.current_) == std::ranges::end(std::get<tIs>(x.parent_->bases_))) ||
                ...);
      }(std::make_index_sequence<1 + sizeof...(TVs)>{});
    }

    // C++23 26.7.32.3 §27
    friend constexpr auto operator<=>(const Iterator& x, const Iterator& y)
    requires(exposition::all_random_access<tConst, TFirst, TVs...>)
    {
      return x.current_ <=> y.current_;
    }

    // C++23 26.7.32.3 §28
    friend constexpr Iterator operator+(Iterator x, difference_type y)
    requires(exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>)
    {
      return x += y;
    }

    // C++23 26.7.32.3 §29
    friend constexpr Iterator operator+(difference_type x, Iterator y)
    requires(exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>)
    {
      return y += x;
    }

    // C++23 26.7.32.3 §30
    friend constexpr Iterator operator-(Iterator x, difference_type y)
    requires(exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>)
    {
      return x -= y;
    }

    // C++23 26.7.32.3 §31
    friend constexpr difference_type operator-(const Iterator& x, const Iterator& y)
    requires(
      exposition::cartesian_is_sized_sentinel<tConst, std::ranges::iterator_t, TFirst, TVs...>)
    {
      return x.distance_from(y.current_);
    }

    // C++23 26.7.32.3 §32-33
    friend constexpr difference_type operator-(const Iterator& i, std::default_sentinel_t /*s*/)
    requires(
      exposition::cartesian_is_sized_sentinel<tConst, std::ranges::sentinel_t, TFirst, TVs...>)
    {
      std::tuple end_tuple = [&]<std::size_t... tIs>(std::index_sequence<tIs...>) {
        return std::tuple{std::ranges::end(std::get<0>(i.parent_->bases_)),
                          std::ranges::begin(std::get<1 + tIs>(i.parent_->bases_))...};
      }(std::make_index_sequence<sizeof...(TVs)>{});
      return i.distance_from(end_tuple);
    }

    // C++23 26.7.32.3 §34
    friend constexpr difference_type operator-(std::default_sentinel_t s, const Iterator& i)
    requires(
      exposition::cartesian_is_sized_sentinel<tConst, std::ranges::sentinel_t, TFirst, TVs...>)
    {
      return -(i - s);
    }

    // C++23 26.7.32.3 §35
    friend constexpr auto iter_move(const Iterator& i) {
      return exposition::tuple_transform(std::ranges::iter_move, i.current_);
    }

    // C++23 26.7.32.3 §37
    friend constexpr void iter_swap(const Iterator& l, const Iterator& r)
    requires(
      std::indirectly_swappable<std::ranges::iterator_t<exposition::MaybeConst<tConst, TFirst>>> &&
      ... &&
      std::indirectly_swappable<std::ranges::iterator_t<exposition::MaybeConst<tConst, TVs>>>)
    {
      [&]<std::size_t... tIs>(std::index_sequence<tIs...>) {
        (std::ranges::iter_swap(std::get<tIs>(l.current_), std::get<tIs>(r.current_)), ...);
      }(std::make_index_sequence<1 + sizeof...(TVs)>{});
    }

  private:
    using Parent = exposition::MaybeConst<tConst, CartesianProductView>;
    using Current = std::tuple<std::ranges::iterator_t<exposition::MaybeConst<tConst, TFirst>>,
                               std::ranges::iterator_t<exposition::MaybeConst<tConst, TVs>>...>;
    friend CartesianProductView;

    // C++23 26.7.32.3 §10
    constexpr Iterator(Parent& parent, Current current)
        : parent_(std::addressof(parent)), current_(std::move(current)) {}

    // C++23 26.7.32.3 §4-5
    template<std::size_t tN = sizeof...(TVs)>
    constexpr void next() {
      auto& it = std::get<tN>(current_);
      ++it;
      if constexpr (tN > 0) {
        if (it == std::ranges::end(std::get<tN>(parent_->bases_))) {
          it = std::ranges::begin(std::get<tN>(parent_->bases_));
          next<tN - 1>();
        }
      }
    }

    // C++23 26.7.32.3 §6
    template<std::size_t tN = sizeof...(TVs)>
    constexpr void prev() {
      auto& it = std::get<tN>(current_);
      if constexpr (tN > 0) {
        if (it == std::ranges::begin(std::get<tN>(parent_->bases_))) {
          it = exposition::cartesian_common_arg_end(std::get<tN>(parent_->bases_));
          prev<tN - 1>();
        }
      }
      --it;
    }

    // Compile-time recursive implementation, constant-time of C++23 26.7.32.3 §18-22
    template<std::size_t tN = sizeof...(TVs)>
    constexpr void advance(difference_type delta)
    requires(exposition::cartesian_product_is_random_access<tConst, TFirst, TVs...>)
    {
      // The implementation borrows the overall approach from the libstdc++, but the documentation
      // is better (a more thorough derivation of the algorithm is in the companion Markdown file).
      // More efficient handling for increment/decrement
      if (delta == 1) {
        next<tN>();
      } else if (delta == -1) {
        prev<tN>();
      } else if (delta != 0) {
        auto& r = std::get<tN>(parent_->bases_);
        auto& it = std::get<tN>(current_);
        if constexpr (tN == 0) {
          it += delta;
        } else {
          const auto size = std::ranges::ssize(r);
          const auto begin = std::ranges::begin(r);
          const auto dim_coord = it - begin; // p_i
          const auto dim_offset = dim_coord + delta; // p_i + δ
          auto dim_new = dim_offset % size; // q_i
          auto rec_offset = dim_offset / size; // δ for the next recursion
          // ensure a non-negative q_i and that the quotient is rounded downwards
          if (dim_new < 0) {
            dim_new = size + dim_new;
            --rec_offset;
          }
          it = begin + dim_new; // the iterator corresponding to q_i
          advance<tN - 1>(rec_offset); // the recursive step
        }
      }
    }

    // C++23 26.7.32.3 §7.1
    template<std::size_t tNum>
    [[nodiscard]] constexpr difference_type scaled_size() const {
      if constexpr (tNum <= sizeof...(TVs)) {
        return static_cast<difference_type>(std::ranges::size(std::get<tNum>(parent_->bases_))) *
               scaled_size<tNum + 1>();
      } else {
        return static_cast<difference_type>(1);
      }
    }

    // C++23 26.7.32.3 §7.2
    template<std::size_t tNum, typename TTuple>
    [[nodiscard]] constexpr difference_type scaled_distance(const TTuple& t) const {
      return static_cast<difference_type>(std::get<tNum>(current_) - std::get<tNum>(t)) *
             scaled_size<tNum + 1>();
    }

    // C++23 26.7.32.3 §7-9
    template<typename TTuple>
    [[nodiscard]] constexpr difference_type distance_from(const TTuple& t) const {
      return [&]<std::size_t... tIs>(std::index_sequence<tIs...>) {
        return (scaled_distance<tIs>(t) + ...);
      }(std::make_index_sequence<1 + sizeof...(TVs)>{});
    }

    Parent* parent_ = nullptr;
    Current current_;
  };

  CartesianProductView() = default;

  // C++23 26.7.32.2 §1
  explicit constexpr CartesianProductView(TFirst first, TVs... rest)
      : bases_(std::move(first), std::move(rest)...) {}

  // C++23 26.7.32.2 §2
  [[nodiscard]] constexpr Iterator<false> begin()
  requires(!exposition::simple_view<TFirst> || ... || !exposition::simple_view<TVs>)
  {
    return Iterator<false>(*this, exposition::tuple_transform(std::ranges::begin, bases_));
  }

  // C++23 26.7.32.2 §3
  [[nodiscard]] constexpr Iterator<true> begin() const
  requires(std::ranges::range<const TFirst> && ... && std::ranges::range<const TVs>)
  {
    return Iterator<true>(*this, exposition::tuple_transform(std::ranges::begin, bases_));
  }

  // C++23 26.7.32.2 §4-5
  [[nodiscard]] constexpr Iterator<false> end()
  requires((!exposition::simple_view<TFirst> || ... || !exposition::simple_view<TVs>) &&
           exposition::cartesian_product_is_common<TFirst, TVs...>)
  {
    auto its = [this]<std::size_t... tIs>(std::index_sequence<tIs...>) {
      using Ret = std::tuple<std::ranges::iterator_t<TFirst>, std::ranges::iterator_t<TVs>...>;
      bool is_empty = (std::ranges::empty(std::get<1 + tIs>(bases_)) || ...);
      auto& first = std::get<0>(bases_);
      return Ret{
        (is_empty ? std::ranges::begin(first) : exposition::cartesian_common_arg_end(first)),
        std::ranges::begin(std::get<1 + tIs>(bases_))...};
    }(std::make_index_sequence<sizeof...(TVs)>{});

    return Iterator<false>{*this, std::move(its)};
  }

  // C++23 26.7.32.2 §4-5
  [[nodiscard]] constexpr Iterator<true> end() const
  requires(exposition::cartesian_product_is_common<const TFirst, const TVs...>)
  {
    auto its = [this]<std::size_t... tIs>(std::index_sequence<tIs...>) {
      using Ret =
        std::tuple<std::ranges::iterator_t<const TFirst>, std::ranges::iterator_t<const TVs>...>;
      bool is_empty = (std::ranges::empty(std::get<1 + tIs>(bases_)) || ...);
      auto& first = std::get<0>(bases_);
      return Ret{
        (is_empty ? std::ranges::begin(first) : exposition::cartesian_common_arg_end(first)),
        std::ranges::begin(std::get<1 + tIs>(bases_))...};
    }(std::make_index_sequence<sizeof...(TVs)>{});

    return Iterator<true>{*this, std::move(its)};
  }

  // C++23 26.7.32.2 §6
  [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept {
    return std::default_sentinel;
  }

  // C++23 26.7.32.2 §7-11
  [[nodiscard]] constexpr auto size()
  requires(exposition::cartesian_product_is_sized<TFirst, TVs...>)
  {
    using UnsignedDiff = MakeUnsigned<difference_type>;
    return [&]<std::size_t... tIs>(std::index_sequence<tIs...>) {
      return (static_cast<UnsignedDiff>(std::ranges::size(std::get<tIs>(bases_))) * ...);
    }(std::make_index_sequence<1 + sizeof...(TVs)>{});
  }

  // C++23 26.7.32.2 §7-11
  [[nodiscard]] constexpr auto size() const
  requires(exposition::cartesian_product_is_sized<const TFirst, const TVs...>)
  {
    using UnsignedDiff = MakeUnsigned<difference_type>;
    return [&]<std::size_t... tIs>(std::index_sequence<tIs...>) {
      return (static_cast<UnsignedDiff>(std::ranges::size(std::get<tIs>(bases_))) * ...);
    }(std::make_index_sequence<1 + sizeof...(TVs)>{});
  }

private:
  std::tuple<TFirst, TVs...> bases_;

  // C++23 26.7.32.3 §3 recommends this to be a type sufficient to represent the product
  // of the sizes, but that seems impractical
  using difference_type = std::make_signed_t<std::size_t>;
};

template<typename... TVs>
CartesianProductView(TVs&&...) -> CartesianProductView<std::views::all_t<TVs>...>;
} // namespace ranges

namespace views {
// Based on C++23 26.7.32.1
struct CartesianProduct {
  template<typename... TEs>
  requires(sizeof...(TEs) == 0 ||
           requires {
             ranges::CartesianProductView<std::views::all_t<TEs>...>(std::declval<TEs>()...);
           })
  constexpr auto operator() [[nodiscard]] (TEs&&... es) const {
    if constexpr (sizeof...(TEs) == 0) {
      return std::views::single(std::tuple{});
    } else {
      return ranges::CartesianProductView<std::views::all_t<TEs>...>(std::forward<TEs>(es)...);
    }
  }
};

inline constexpr CartesianProduct cartesian_product;
} // namespace views
#endif
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_CARTESIAN_PRODUCT_HPP
