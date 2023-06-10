#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP

#include <cstddef>
#include <functional>

#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/definitions/type-traits.hpp"
#include "thesauros/utility/static-ranges/ranges/filter.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/static-ranges/ranges/transform.hpp"
#include "thesauros/utility/static-ranges/sinks/reduce.hpp"

namespace thes::star {
template<typename TRange>
inline constexpr auto postfix_product_inclusive(const TRange& range) {
  using Value = star::Value<TRange>;
  constexpr std::size_t size = thes::star::size<TRange>;

  return iota<0, size + 1> | transform([&range](auto idx) {
           return range | star::only_idxseq<iota<idx, size>> |
                  left_reduce(std::multiplies<>{}, Value{1});
         });
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP
