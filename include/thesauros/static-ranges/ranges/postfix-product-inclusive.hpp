#ifndef INCLUDE_THESAUROS_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP

#include <cstddef>
#include <functional>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/ranges/filter.hpp"
#include "thesauros/static-ranges/ranges/iota.hpp"
#include "thesauros/static-ranges/ranges/transform.hpp"
#include "thesauros/static-ranges/sinks/reduce.hpp"

namespace thes::star {
template<typename TRange>
THES_ALWAYS_INLINE inline constexpr auto postfix_product_inclusive(const TRange& range) {
  using Value = star::Value<TRange>;
  constexpr std::size_t size = thes::star::size<TRange>;

  return transform([&range](auto idx) THES_ALWAYS_INLINE {
    return static_cast<Value>(
      left_reduce(std::multiplies<>{}, Value{1})(star::only_range<iota<idx, size>>(range)));
  })(iota<0, size + 1>);
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_RANGES_POSTFIX_PRODUCT_INCLUSIVE_HPP
