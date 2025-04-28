#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_TUPLE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_TUPLE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
struct ToTupleGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxd*/) {
      return Tuple{get_at<tIdxs>(range)...};
    }(std::make_index_sequence<size<TRange>>{});
  }
};

inline constexpr ToTupleGenerator to_tuple{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_TUPLE_HPP
