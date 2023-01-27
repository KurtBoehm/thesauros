#ifndef INCLUDE_THESAUROS_ITERATOR_CONCEPTS_HPP
#define INCLUDE_THESAUROS_ITERATOR_CONCEPTS_HPP

#include <compare>
#include <concepts>

namespace thes {
namespace iterator_provider {
template<typename TState, typename TProvider>
concept Deref = requires(const TState& s) {
                  {
                    TProvider::deref(s)
                    } -> std::convertible_to<typename TProvider::IterTypes::IterRef>;
                };

template<typename TState, typename TProvider>
concept Incr = requires(TState& s) { TProvider::incr(s); };
template<typename TState, typename TProvider>
concept Decr = requires(TState& s) { TProvider::decr(s); };

template<typename TState, typename TProvider>
concept Eq = requires(const TState& s1, const TState& s2) {
               { TProvider::eq(s1, s2) } -> std::convertible_to<bool>;
             };
template<typename TState, typename TProvider>
concept ThreeWayCmp = requires(const TState& s1, const TState& s2) {
                        {
                          TProvider::three_way(s1, s2)
                          } -> std::convertible_to<std::strong_ordering>;
                      };

template<typename TState, typename TProvider>
concept InPlaceAdd =
  requires(TState& s, typename TProvider::IterTypes::IterDiff d) { TProvider::iadd(s, d); };
template<typename TState, typename TProvider>
concept InPlaceSub =
  requires(TState& s, typename TProvider::IterTypes::IterDiff d) { TProvider::isub(s, d); };

template<typename TState, typename TProvider>
concept Sub = requires(const TState& s1, const TState& s2) {
                {
                  TProvider::sub(s1, s2)
                  } -> std::convertible_to<typename TProvider::IterTypes::IterDiff>;
              };

template<typename TState, typename TProvider>
concept GetItem = requires(const TState& s, typename TProvider::IterTypes::IterDiff d) {
                    {
                      TProvider::get_item(s, d)
                      } -> std::convertible_to<typename TProvider::IterTypes::IterRef>;
                  };

template<typename TState, typename TProvider>
concept ForwardIterProvider =
  Deref<TState, TProvider> && Incr<TState, TProvider> && Eq<TState, TProvider>;

template<typename TState, typename TProvider>
concept BidirectionalIterProvider =
  ForwardIterProvider<TState, TProvider> && Decr<TState, TProvider>;

template<typename TState, typename TProvider>
concept RandomAccessIterProvider =
  BidirectionalIterProvider<TState, TProvider> && InPlaceAdd<TState, TProvider> &&
  Sub<TState, TProvider> && ThreeWayCmp<TState, TProvider>;

template<typename TProvider, typename TIt1, typename TIt2>
concept TestIfCmp = requires(const TIt1& i1, const TIt2& i2) { TProvider::test_if_cmp(i1, i2); };

template<typename TState, typename TProvider>
concept RevDeref = requires(const TState& s) {
                     {
                       TProvider::rev_deref(s)
                       } -> std::convertible_to<typename TProvider::IterTypes::IterRef>;
                   };
} // namespace iterator_provider

namespace iterator {
template<typename TIt>
concept IncrAny = requires(TIt& v) { ++v; };
template<typename TIt>
concept DecrAny = requires(TIt& v) { --v; };
template<typename TIt, typename TDiff>
concept InPlaceAdd = requires(TIt& v, TDiff d) { v += d; };
template<typename TIt, typename TDiff>
concept InPlaceSub = requires(TIt& v, TDiff d) { v -= d; };
template<typename TIt>
concept Eq = requires(const TIt& v1, const TIt& v2) {
               { v1 == v2 } -> std::convertible_to<bool>;
             };
template<typename TIt>
concept ThreeWayCmp = requires(const TIt& v1, const TIt& v2) {
                        { v1 <=> v2 } -> std::convertible_to<std::strong_ordering>;
                      };
template<typename TIt, typename TDiff>
concept Sub = std::integral<TDiff> || requires(const TIt& v1, const TIt& v2) {
                                        { v1 - v2 } -> std::convertible_to<TDiff>;
                                      };
} // namespace iterator
} // namespace thes

#endif // INCLUDE_THESAUROS_ITERATOR_CONCEPTS_HPP
