#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_GET_AT_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

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
  static constexpr decltype(auto) get_at(const TRange& c) {
    return c.template get<tIndex>();
  }
  static constexpr decltype(auto) get_at(TRange& c) {
    return c.template get<tIndex>();
  }
};

template<std::size_t tIndex, typename TRange>
requires(!impl::HasMemberGet<tIndex, TRange> && impl::HasFreeGet<tIndex, TRange>)
struct GetAtTrait<tIndex, TRange> {
  static constexpr decltype(auto) get_at(const TRange& array) {
    return std::get<tIndex>(array);
  }
  static constexpr decltype(auto) get_at(TRange& array) {
    return std::get<tIndex>(array);
  }
};

template<std::size_t tIndex, typename TRange>
requires(requires { sizeof(GetAtTrait<tIndex, std::decay_t<TRange>>); })
inline constexpr decltype(auto) get_at(TRange& c) {
  return GetAtTrait<tIndex, std::decay_t<TRange>>::get_at(c);
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
