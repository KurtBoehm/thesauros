#ifndef INCLUDE_THESAUROS_STATIC_RANGES_RANGES_ZIP_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_RANGES_ZIP_HPP

#include <array>
#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/sinks/apply.hpp"
#include "thesauros/static-ranges/sinks/unique-value.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
template<typename... TRanges>
struct ZipView {
  Tuple<TRanges...> ranges;

  static constexpr std::size_t size = *unique_value(std::array{thes::star::size<TRanges>...});

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE constexpr auto get() const {
    return apply([](auto&... inner) THES_ALWAYS_INLINE {
      return Tuple{thes::star::get_at<tIndex>(inner)...};
    })(ranges);
  }
};

template<typename... TRanges>
requires(sizeof...(TRanges) > 0)
THES_ALWAYS_INLINE inline constexpr auto zip(TRanges&&... ranges) {
  return ZipView<TRanges...>{Tuple{std::forward<TRanges>(ranges)...}};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_RANGES_ZIP_HPP
