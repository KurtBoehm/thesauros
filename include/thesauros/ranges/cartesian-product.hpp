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
template<std::ranges::input_range First, std::ranges::forward_range... V>
using CartesianProductView = std::ranges::cartesian_product_view<First, V...>;
} // namespace ranges
namespace views {
inline constexpr auto cartesian_product = std::views::cartesian_product;
} // namespace views
#else
namespace ranges {
// C++23 26.7.32.2
template<std::ranges::input_range First, std::ranges::forward_range... V>
requires(std::ranges::view<First> && ... && std::ranges::view<V>)
struct CartesianProductView
    : public std::ranges::view_interface<CartesianProductView<First, V...>> {
  template<bool Const>
  struct Iterator {
    using iterator_category = std::input_iterator_tag;
    // C++23 26.7.32.3 §1
    using iterator_concept = decltype([] {
      if constexpr (exposition::cartesian_product_is_random_access<Const, First, V...>) {
        return std::random_access_iterator_tag{};
      } else if constexpr (exposition::cartesian_product_is_bidirectional<Const, First, V...>) {
        return std::bidirectional_iterator_tag{};
      } else if constexpr (std::ranges::forward_range<exposition::MaybeConst<Const, First>>) {
        return std::forward_iterator_tag{};
      } else {
        return std::input_iterator_tag{};
      }
    }());
    using value_type = std::tuple<std::ranges::range_value_t<exposition::MaybeConst<Const, First>>,
                                  std::ranges::range_value_t<exposition::MaybeConst<Const, V>>...>;
    using reference =
      std::tuple<std::ranges::range_reference_t<exposition::MaybeConst<Const, First>>,
                 std::ranges::range_reference_t<exposition::MaybeConst<Const, V>>...>;
    // C++23 26.7.32.3 §2
    using difference_type = CartesianProductView::difference_type;

    Iterator() = default;

    // C++23 26.7.32.3 §11
    constexpr Iterator(Iterator<!Const> i)
    requires(Const &&
             (std::convertible_to<std::ranges::iterator_t<First>,
                                  std::ranges::iterator_t<const First>> &&
              ... &&
              std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<const V>>))
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
    requires(std::ranges::forward_range<exposition::MaybeConst<Const, First>>)
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    // C++23 26.7.32.3 §16
    constexpr Iterator& operator--()
    requires(exposition::cartesian_product_is_bidirectional<Const, First, V...>)
    {
      prev();
      return *this;
    }

    // C++23 26.7.32.3 §17
    constexpr Iterator operator--(int)
    requires(exposition::cartesian_product_is_bidirectional<Const, First, V...>)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    // C++23 26.7.32.3 §18-22
    constexpr Iterator& operator+=(difference_type x)
    requires(exposition::cartesian_product_is_random_access<Const, First, V...>)
    {
      // Outsourced to a fancy, template-recursive implementation
      advance(x);
      return *this;
    }

    // C++23 26.7.32.3 §23
    constexpr Iterator& operator-=(difference_type x)
    requires(exposition::cartesian_product_is_random_access<Const, First, V...>)
    {
      return *this += -x;
    }

    // C++23 26.7.32.3 §24
    constexpr reference operator[](difference_type n) const
    requires(exposition::cartesian_product_is_random_access<Const, First, V...>)
    {
      return *((*this) + n);
    }

    // C++23 26.7.32.3 §25
    friend constexpr bool operator==(const Iterator& x, const Iterator& y)
    requires(
      std::equality_comparable<std::ranges::iterator_t<exposition::MaybeConst<Const, First>>>)
    {
      return x.current_ == y.current_;
    }

    // C++23 26.7.32.3 §26
    friend constexpr bool operator==(const Iterator& x, std::default_sentinel_t) {
      return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return ((std::get<I>(x.current_) == std::ranges::end(std::get<I>(x.parent_->bases_))) ||
                ...);
      }(std::make_index_sequence<1 + sizeof...(V)>{});
    }

    // C++23 26.7.32.3 §27
    friend constexpr auto operator<=>(const Iterator& x, const Iterator& y)
    requires(exposition::all_random_access<Const, First, V...>)
    {
      return x.current_ <=> y.current_;
    }

    // C++23 26.7.32.3 §28
    friend constexpr Iterator operator+(Iterator x, difference_type y)
    requires(exposition::cartesian_product_is_random_access<Const, First, V...>)
    {
      return x += y;
    }

    // C++23 26.7.32.3 §29
    friend constexpr Iterator operator+(difference_type x, Iterator y)
    requires(exposition::cartesian_product_is_random_access<Const, First, V...>)
    {
      return y += x;
    }

    // C++23 26.7.32.3 §30
    friend constexpr Iterator operator-(Iterator x, difference_type y)
    requires(exposition::cartesian_product_is_random_access<Const, First, V...>)
    {
      return x -= y;
    }

    // C++23 26.7.32.3 §31
    friend constexpr difference_type operator-(const Iterator& x, const Iterator& y)
    requires(exposition::cartesian_is_sized_sentinel<Const, std::ranges::iterator_t, First, V...>)
    {
      return x.distance_from(y.current_);
    }

    // C++23 26.7.32.3 §32-33
    friend constexpr difference_type operator-(const Iterator& i, std::default_sentinel_t /*s*/)
    requires(exposition::cartesian_is_sized_sentinel<Const, std::ranges::sentinel_t, First, V...>)
    {
      std::tuple end_tuple = [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::tuple{std::ranges::end(std::get<0>(i.parent_->bases_)),
                          std::ranges::begin(std::get<1 + I>(i.parent_->bases_))...};
      }(std::make_index_sequence<sizeof...(V)>{});
      return i.distance_from(end_tuple);
    }

    // C++23 26.7.32.3 §34
    friend constexpr difference_type operator-(std::default_sentinel_t s, const Iterator& i)
    requires(exposition::cartesian_is_sized_sentinel<Const, std::ranges::sentinel_t, First, V...>)
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
      std::indirectly_swappable<std::ranges::iterator_t<exposition::MaybeConst<Const, First>>> &&
      ... && std::indirectly_swappable<std::ranges::iterator_t<exposition::MaybeConst<Const, V>>>)
    {
      [&]<std::size_t... I>(std::index_sequence<I...>) {
        (std::ranges::iter_swap(std::get<I>(l.current_), std::get<I>(r.current_)), ...);
      }(std::make_index_sequence<1 + sizeof...(V)>{});
    }

  private:
    using Parent = exposition::MaybeConst<Const, CartesianProductView>;
    using Current = std::tuple<std::ranges::iterator_t<exposition::MaybeConst<Const, First>>,
                               std::ranges::iterator_t<exposition::MaybeConst<Const, V>>...>;
    friend CartesianProductView;

    // C++23 26.7.32.3 §10
    constexpr Iterator(Parent& parent, Current current)
        : parent_(std::addressof(parent)), current_(std::move(current)) {}

    // C++23 26.7.32.3 §4-5
    template<std::size_t N = sizeof...(V)>
    constexpr void next() {
      auto& it = std::get<N>(current_);
      ++it;
      if constexpr (N > 0) {
        if (it == std::ranges::end(std::get<N>(parent_->bases_))) {
          it = std::ranges::begin(std::get<N>(parent_->bases_));
          next<N - 1>();
        }
      }
    }

    // C++23 26.7.32.3 §6
    template<std::size_t N = sizeof...(V)>
    constexpr void prev() {
      auto& it = std::get<N>(current_);
      if constexpr (N > 0) {
        if (it == std::ranges::begin(std::get<N>(parent_->bases_))) {
          it = exposition::cartesian_common_arg_end(std::get<N>(parent_->bases_));
          prev<N - 1>();
        }
      }
      --it;
    }

    // Compile-time recursive implementation, constant-time of C++23 26.7.32.3 §18-22
    template<std::size_t N = sizeof...(V)>
    constexpr void advance(difference_type delta)
    requires(exposition::cartesian_product_is_random_access<Const, First, V...>)
    {
      // The implementation borrows the overall approach from the libstdc++, but the documentation
      // is better (a more thorough derivation of the algorithm is in the companion Markdown file).
      // More efficient handling for increment/decrement
      if (delta == 1) {
        next<N>();
      } else if (delta == -1) {
        prev<N>();
      } else if (delta != 0) {
        auto& r = std::get<N>(parent_->bases_);
        auto& it = std::get<N>(current_);
        if constexpr (N == 0) {
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
          advance<N - 1>(rec_offset); // the recursive step
        }
      }
    }

    // C++23 26.7.32.3 §7.1
    template<std::size_t N>
    [[nodiscard]] constexpr difference_type scaled_size() const {
      if constexpr (N <= sizeof...(V)) {
        return static_cast<difference_type>(std::ranges::size(std::get<N>(parent_->bases_))) *
               scaled_size<N + 1>();
      } else {
        return static_cast<difference_type>(1);
      }
    }

    // C++23 26.7.32.3 §7.2
    template<std::size_t N, typename TTuple>
    [[nodiscard]] constexpr difference_type scaled_distance(const TTuple& t) const {
      return static_cast<difference_type>(std::get<N>(current_) - std::get<N>(t)) *
             scaled_size<N + 1>();
    }

    // C++23 26.7.32.3 §7-9
    template<typename TTuple>
    [[nodiscard]] constexpr difference_type distance_from(const TTuple& t) const {
      return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return (scaled_distance<I>(t) + ...);
      }(std::make_index_sequence<1 + sizeof...(V)>{});
    }

    Parent* parent_ = nullptr;
    Current current_;
  };

  CartesianProductView() = default;

  // C++23 26.7.32.2 §1
  explicit constexpr CartesianProductView(First first, V... rest)
      : bases_(std::move(first), std::move(rest)...) {}

  // C++23 26.7.32.2 §2
  [[nodiscard]] constexpr Iterator<false> begin()
  requires(!exposition::simple_view<First> || ... || !exposition::simple_view<V>)
  {
    return Iterator<false>(*this, exposition::tuple_transform(std::ranges::begin, bases_));
  }

  // C++23 26.7.32.2 §3
  [[nodiscard]] constexpr Iterator<true> begin() const
  requires(std::ranges::range<const First> && ... && std::ranges::range<const V>)
  {
    return Iterator<true>(*this, exposition::tuple_transform(std::ranges::begin, bases_));
  }

  // C++23 26.7.32.2 §4-5
  [[nodiscard]] constexpr Iterator<false> end()
  requires((!exposition::simple_view<First> || ... || !exposition::simple_view<V>) &&
           exposition::cartesian_product_is_common<First, V...>)
  {
    auto its = [this]<std::size_t... I>(std::index_sequence<I...>) {
      using Ret = std::tuple<std::ranges::iterator_t<First>, std::ranges::iterator_t<V>...>;
      bool is_empty = (std::ranges::empty(std::get<1 + I>(bases_)) || ...);
      auto& first = std::get<0>(bases_);
      return Ret{
        (is_empty ? std::ranges::begin(first) : exposition::cartesian_common_arg_end(first)),
        std::ranges::begin(std::get<1 + I>(bases_))...};
    }(std::make_index_sequence<sizeof...(V)>{});

    return Iterator<false>{*this, std::move(its)};
  }

  // C++23 26.7.32.2 §4-5
  [[nodiscard]] constexpr Iterator<true> end() const
  requires(exposition::cartesian_product_is_common<const First, const V...>)
  {
    auto its = [this]<std::size_t... I>(std::index_sequence<I...>) {
      using Ret =
        std::tuple<std::ranges::iterator_t<const First>, std::ranges::iterator_t<const V>...>;
      bool is_empty = (std::ranges::empty(std::get<1 + I>(bases_)) || ...);
      auto& first = std::get<0>(bases_);
      return Ret{
        (is_empty ? std::ranges::begin(first) : exposition::cartesian_common_arg_end(first)),
        std::ranges::begin(std::get<1 + I>(bases_))...};
    }(std::make_index_sequence<sizeof...(V)>{});

    return Iterator<true>{*this, std::move(its)};
  }

  // C++23 26.7.32.2 §6
  [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept {
    return std::default_sentinel;
  }

  // C++23 26.7.32.2 §7-11
  [[nodiscard]] constexpr auto size()
  requires(exposition::cartesian_product_is_sized<First, V...>)
  {
    using UnsignedDiff = MakeUnsigned<difference_type>;
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
      return (static_cast<UnsignedDiff>(std::ranges::size(std::get<I>(bases_))) * ...);
    }(std::make_index_sequence<1 + sizeof...(V)>{});
  }

  // C++23 26.7.32.2 §7-11
  [[nodiscard]] constexpr auto size() const
  requires(exposition::cartesian_product_is_sized<const First, const V...>)
  {
    using UnsignedDiff = MakeUnsigned<difference_type>;
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
      return (static_cast<UnsignedDiff>(std::ranges::size(std::get<I>(bases_))) * ...);
    }(std::make_index_sequence<1 + sizeof...(V)>{});
  }

private:
  std::tuple<First, V...> bases_;

  // C++23 26.7.32.3 §3 recommends this to be a type sufficient to represent the product
  // of the sizes, but that seems impractical
  using difference_type = std::make_signed_t<std::size_t>;
};

template<typename... V>
CartesianProductView(V&&...) -> CartesianProductView<std::views::all_t<V>...>;
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
