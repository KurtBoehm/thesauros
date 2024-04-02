#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FIRST_VALUE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FIRST_VALUE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/definitions/type-traits.hpp"
#include "thesauros/utility/void-macros.hpp"

namespace thes::star {
struct FirstValueGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    using Ret = Value<TRange>;
    return work<Ret>(range, std::make_index_sequence<size>{});
  }

  template<typename TRet, std::size_t tHead, std::size_t... tTail>
  THES_ALWAYS_INLINE constexpr decltype(auto)
  work(auto& range, std::index_sequence<tHead, tTail...> /*idxs*/) const {
    THES_APPLY_VALUED_RETURN(TRet, get_at<tHead>(range));
    if constexpr (sizeof...(tTail) > 0) {
      return work<TRet>(range, std::index_sequence<tTail...>{});
    } else {
      THES_RETURN_EMPTY_OPTIONAL(TRet);
    }
  }
};

inline constexpr FirstValueGenerator first_value{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FIRST_VALUE_HPP
