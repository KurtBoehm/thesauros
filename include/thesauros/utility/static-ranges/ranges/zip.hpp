#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_ZIP_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_ZIP_HPP

#include <array>
#include <cstddef>

#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/sinks/apply.hpp"
#include "thesauros/utility/static-ranges/sinks/unique-value.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
template<typename... TRanges>
struct ZipView {
  thes::Tuple<TRanges...> ranges;

  static constexpr std::size_t size =
    *thes::star::unique_value(std::array{thes::star::size<TRanges>...});

  template<std::size_t tIndex>
  constexpr auto get() const {
    return thes::star::apply(
      [](auto&... inner) { return thes::Tuple{thes::star::get_at<tIndex>(inner)...}; })(ranges);
  }
};

template<typename... TRanges>
requires(sizeof...(TRanges) > 0)
inline constexpr auto zip(TRanges&&... ranges) {
  return ZipView<TRanges...>{Tuple{std::forward<TRanges>(ranges)...}};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_ZIP_HPP
