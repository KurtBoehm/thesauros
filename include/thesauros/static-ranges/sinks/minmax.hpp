#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_MINMAX_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_MINMAX_HPP

#include <cstddef>

#include "thesauros/static-ranges/definitions.hpp"
#include "thesauros/static-ranges/ranges/iota.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes::star {
struct MinMaxGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  requires(star::size<TRange> > 0 && HasValue<TRange>)
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = star::size<TRange>;
    using Value = star::Value<TRange>;
    Value min = get_at(range, thes::index_tag<0>);
    Value max = min;
    thes::star::iota<1, size> | thes::star::for_each([&](auto i) {
      Value c = get_at(range, i);
      min = std::min(min, c);
      max = std::max(max, c);
    });
    return std::make_pair(std::move(min), std::move(max));
  }
};

inline constexpr MinMaxGenerator minmax{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_MINMAX_HPP
