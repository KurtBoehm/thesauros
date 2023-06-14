#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FIRST_VALUE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FIRST_VALUE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/type-sequence.hpp"
#include "thesauros/utility/void-macros.hpp"

namespace thes::star {
template<typename TOp>
struct FirstValueGenerator : public ConsumerGeneratorBase {
  template<typename TRange, typename TIdxSeq>
  struct RetImpl;
  template<typename TRange, std::size_t... tIdxs>
  struct RetImpl<TRange, std::index_sequence<tIdxs...>> {
    using Type =
      TypeSeq<decltype(std::declval<TOp>()(get_at<tIdxs>(std::declval<TRange&>())))...>::Unique;
  };

  TOp op;

  explicit constexpr FirstValueGenerator(TOp&& op) : op(std::forward<TOp>(op)) {}

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    using Ret = RetImpl<TRange, std::make_index_sequence<size>>::Type;
    return work<Ret>(range, std::make_index_sequence<size>{});
  }

  template<typename TRet, std::size_t tHead, std::size_t... tTail>
  constexpr decltype(auto) work(auto& range, std::index_sequence<tHead, tTail...> /*idxs*/) const {
    THES_APPLY_VALUED_RETURN(TRet, op(get_at<tHead>(range)));
    if constexpr (sizeof...(tTail) > 0) {
      return work<TRet>(range, std::index_sequence<tTail...>{});
    } else {
      THES_RETURN_EMPTY_OPTIONAL(TRet);
    }
  }
};

template<typename TOp>
inline constexpr auto first_value(TOp&& op) {
  return FirstValueGenerator<TOp>{std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FIRST_VALUE_HPP
