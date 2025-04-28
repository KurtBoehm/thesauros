#ifndef INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_TYPE_SEQUENCE_HPP
#define INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_TYPE_SEQUENCE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <variant>

#include "thesauros/utility/tuple.hpp"

namespace thes {
template<typename... Ts>
struct TypeSeqBase {
  static constexpr bool is_unique = false;
};
template<typename THead, typename... TTail>
requires(... && std::same_as<THead, TTail>)
struct TypeSeqBase<THead, TTail...> {
  static constexpr bool is_unique = true;
  using Unique = THead;
};

template<typename... Ts>
struct TypeSeq : public TypeSeqBase<Ts...> {
  using AsTuple = Tuple<Ts...>;

  template<std::size_t tIndex>
  using At = TupleElement<tIndex, AsTuple>;

  template<typename T>
  static constexpr bool contains = (... || std::same_as<T, Ts>);

  template<typename T>
  using Prepended = TypeSeq<T, Ts...>;

  template<typename... TOthers>
  inline constexpr TypeSeq<TOthers..., Ts...> prepend(TypeSeq<TOthers...> /*seq*/) const {
    return {};
  }
};

template<typename T>
struct IsTypeSeqTrait : public std::false_type {};
template<typename... Ts>
struct IsTypeSeqTrait<TypeSeq<Ts...>> : public std::true_type {};
template<typename T>
concept AnyTypeSeq = IsTypeSeqTrait<T>::value;

inline consteval bool operator==(AnyTypeSeq auto seq1, AnyTypeSeq auto seq2) {
  return std::same_as<decltype(seq1), decltype(seq2)>;
}

namespace impl {
template<typename T>
struct AsTypeSeq {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct AsTypeSeq<TypeSeq<Ts...>> {
  using Type = TypeSeq<Ts...>;
};
} // namespace impl
template<typename T>
using AsTypeSeq = impl::AsTypeSeq<T>::Type;

namespace impl {
template<typename T>
struct ConvertedTypeSeq {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct ConvertedTypeSeq<std::variant<Ts...>> {
  using Type = TypeSeq<typename ConvertedTypeSeq<Ts>::Type...>;
};
template<typename... Ts>
struct ConvertedTypeSeq<TypeSeq<Ts...>> {
  using Type = TypeSeq<typename ConvertedTypeSeq<Ts>::Type...>;
};
} // namespace impl
template<typename T>
using ConvertedTypeSeq = impl::ConvertedTypeSeq<T>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_TYPE_SEQUENCE_HPP
