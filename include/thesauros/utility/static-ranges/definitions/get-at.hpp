#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_GET_AT_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "thesauros/utility/value-tag.hpp"

namespace thes::star {
namespace impl {
template<std::size_t tIndex, typename TRange>
concept HasMemberGet = requires(TRange&& rng) { std::forward<TRange>(rng).template get<tIndex>(); };

template<std::size_t tIndex, typename TRange>
concept HasFreeGet = requires(TRange&& rng) { std::get<tIndex>(std::forward<TRange>(rng)); };
} // namespace impl

template<std::size_t tIndex, typename TRange>
struct GetAtTrait;

template<std::size_t tIndex, typename TRange>
requires impl::HasMemberGet<tIndex, TRange>
struct GetAtTrait<tIndex, TRange> {
  static constexpr decltype(auto) get_at(TRange&& range) {
    return std::forward<TRange>(range).template get<tIndex>();
  }
};

template<std::size_t tIndex, typename TRange>
requires(!impl::HasMemberGet<tIndex, TRange> && impl::HasFreeGet<tIndex, TRange>)
struct GetAtTrait<tIndex, TRange> {
  static constexpr decltype(auto) get_at(TRange&& range) {
    return std::get<tIndex>(std::forward<TRange>(range));
  }
};

template<std::size_t tIndex, typename TRange>
requires(requires { sizeof(GetAtTrait<tIndex, TRange>); })
inline constexpr decltype(auto) get_at(TRange&& r, IndexTag<tIndex> /*tag*/ = {}) {
  return GetAtTrait<tIndex, TRange>::get_at(std::forward<TRange>(r));
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
