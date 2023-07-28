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
concept HasMemberGet = requires(const TRange& c) { c.template get<tIndex>(); };

template<std::size_t tIndex, typename TRange>
concept HasFreeGet = requires(const TRange& c) { std::get<tIndex>(c); };
} // namespace impl

template<std::size_t tIndex, typename TRange>
struct GetAtTrait;

template<std::size_t tIndex, typename TRange>
requires impl::HasMemberGet<tIndex, TRange>
struct GetAtTrait<tIndex, TRange> {
  template<typename TR>
  requires std::same_as<std::decay_t<TR>, TRange>
  static constexpr decltype(auto) get_at(TR&& range) {
    return std::forward<TR>(range).template get<tIndex>();
  }
};

template<std::size_t tIndex, typename TRange>
requires(!impl::HasMemberGet<tIndex, TRange> && impl::HasFreeGet<tIndex, TRange>)
struct GetAtTrait<tIndex, TRange> {
  template<typename TR>
  requires std::same_as<std::decay_t<TR>, TRange>
  static constexpr decltype(auto) get_at(TR&& range) {
    return std::get<tIndex>(std::forward<TR>(range));
  }
};

template<std::size_t tIndex, typename TRange>
requires(requires { sizeof(GetAtTrait<tIndex, std::decay_t<TRange>>); })
inline constexpr decltype(auto) get_at(TRange&& r, IndexTag<tIndex> /*tag*/ = {}) {
  return GetAtTrait<tIndex, std::decay_t<TRange>>::get_at(std::forward<TRange>(r));
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
