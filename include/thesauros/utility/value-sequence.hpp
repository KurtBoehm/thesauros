#ifndef INCLUDE_THESAUROS_UTILITY_VALUE_SEQUENCE_HPP
#define INCLUDE_THESAUROS_UTILITY_VALUE_SEQUENCE_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "thesauros/utility/type-sequence.hpp"

namespace thes {
namespace value_seq_impl {
template<typename T, T... tValues>
struct UniqueTrait {
  static constexpr bool is_unique = false;
};

template<typename T, T tHead, T... tTail>
requires(... && (tHead == tTail))
struct UniqueTrait<T, tHead, tTail...> {
  static constexpr bool is_unique = true;
  static constexpr T unique = tHead;
};
} // namespace value_seq_impl

template<typename T, T... tValues>
struct ValueSequence : public value_seq_impl::UniqueTrait<T, tValues...> {
  using Value = T;
  static constexpr std::size_t size = sizeof...(tValues);

  template<std::size_t tIndex>
  static constexpr Value get_at = std::get<tIndex>(std::tuple<decltype(tValues)...>{tValues...});

private:
  template<std::size_t tIdx, typename TExclSeq>
  struct ExceptTrait {
    static constexpr T element = get_at<tIdx>;
    static constexpr bool exclude = TExclSeq::template contains<element>;
    using Suffix = typename ExceptTrait<tIdx + 1, TExclSeq>::Seq;
    using Seq = std::conditional_t<exclude, Suffix, typename Suffix::template Prepend<element>>;
  };
  template<typename TExclSeq>
  struct ExceptTrait<size, TExclSeq> {
    using Seq = ValueSequence<T>;
  };

public:
  template<Value tNew>
  using Prepend = ValueSequence<T, tNew, tValues...>;
  template<Value tNew>
  using Append = ValueSequence<T, tValues..., tNew>;
  template<typename TExclSeq>
  using ExceptSequence = typename ExceptTrait<0, TExclSeq>::Seq;
  template<Value... tExcl>
  using ExceptValues = typename ExceptTrait<0, ValueSequence<T, tExcl...>>::Seq;

  template<T tValue>
  static constexpr bool contains = (... || (tValues == tValue));
};

template<auto... tValues>
requires TypeSequence<decltype(tValues)...>::is_unique
using AutoValueSequence =
  ValueSequence<typename TypeSequence<decltype(tValues)...>::Unique, tValues...>;

template<typename T, T tCurrent, T tEnd>
struct MakeIntegerSequenceTrait {
  using Sequence =
    typename MakeIntegerSequenceTrait<T, tCurrent + 1, tEnd>::Sequence::template Prepend<tCurrent>;
};

template<typename T, T tEnd>
struct MakeIntegerSequenceTrait<T, tEnd, tEnd> {
  using Sequence = ValueSequence<T>;
};

template<typename T, T tBegin, T tEnd>
using MakeIntegerSequence = typename MakeIntegerSequenceTrait<T, tBegin, tEnd>::Sequence;

template<auto tBegin, auto tEnd>
requires TypeSequence<decltype(tBegin), decltype(tEnd)>::is_unique
using AutoMakeIntegerSequence =
  MakeIntegerSequence<typename TypeSequence<decltype(tBegin), decltype(tEnd)>::Unique, tBegin,
                      tEnd>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VALUE_SEQUENCE_HPP
