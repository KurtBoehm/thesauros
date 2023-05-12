#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_TUPLE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_TUPLE_HPP

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/value-sequence.hpp"

namespace thes::star {
struct ToTupleGenerator {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxd*/) {
      return std::tuple{get_at<tIdxs>(range)...};
    }(std::make_index_sequence<size<TRange>>{});
  }
};
template<>
struct ConsumerGeneratorTrait<ToTupleGenerator> : public std::true_type {};

inline constexpr ToTupleGenerator to_tuple{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_TUPLE_HPP
