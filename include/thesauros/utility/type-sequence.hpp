#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>

namespace thes {
namespace type_seq_impl {
template<typename... Ts>
struct UniqueTrait {
  static constexpr bool is_unique = false;
};

template<typename THead, typename... TTail>
requires(... && std::same_as<THead, TTail>)
struct UniqueTrait<THead, TTail...> {
  static constexpr bool is_unique = true;
  using Unique = THead;
};
} // namespace type_seq_impl

template<typename... Ts>
struct TypeSequence : public type_seq_impl::UniqueTrait<Ts...> {
  template<std::size_t tIndex>
  using GetAt = std::tuple_element_t<tIndex, std::tuple<Ts...>>;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP
