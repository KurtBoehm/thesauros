#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FORMAT_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FORMAT_HPP

#include <utility>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"

namespace thes::star {
template<typename TRange>
struct Formatter {
  explicit constexpr Formatter(TRange&& range) : range_(std::forward<TRange>(range)) {}

  constexpr const TRange& range() const {
    return range_;
  }

private:
  TRange range_;
};

struct FormatGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    return Formatter<TRange>(std::forward<TRange>(range));
  }
};

inline constexpr FormatGenerator format{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FORMAT_HPP
