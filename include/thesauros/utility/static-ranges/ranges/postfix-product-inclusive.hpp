#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP

#include "thesauros/utility/static-ranges/definitions/element-type.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/filter.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/static-ranges/ranges/map-values.hpp"
#include "thesauros/utility/static-ranges/sinks/reduce.hpp"
#include "thesauros/utility/value-sequence.hpp"
#include <functional>

namespace thes::star {
template<typename TRange>
inline constexpr auto postfix_product_inclusive(const TRange& range) {
  using Value = ElementType<TRange>;
  constexpr std::size_t size = ::thes::star::size<TRange>;

  return iota<0, size + 1> | map_values([&range](auto idx) {
           using Sequence = MakeIntegerSequence<std::size_t, idx, size>;
           return range | star::only_idxseq<Sequence> | left_reduce(std::multiplies<>{}, Value{1});
         });
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP
