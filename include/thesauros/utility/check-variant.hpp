#ifndef INCLUDE_THESAUROS_UTILITY_CHECK_VARIANT_HPP
#define INCLUDE_THESAUROS_UTILITY_CHECK_VARIANT_HPP

#include <exception>
#include <variant>

#include "thesauros/utility/fancy-visit.hpp"

namespace thes {
template<typename... Ts>
inline constexpr auto check_variant(std::variant<Ts...>&& variant, auto checker) {
  return fancy_filter_visit(
    [&]<typename T>(T&& val) {
      if constexpr (std::derived_from<decltype(checker(val)), std::exception>) {
        throw checker(val);
        return fancy_visitor_ignore;
      } else {
        return std::forward<T>(val);
      }
    },
    variant);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_CHECK_VARIANT_HPP
