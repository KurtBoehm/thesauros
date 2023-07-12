#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_ALL_DIFFERENT_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_ALL_DIFFERENT_HPP

#include <cstddef>
#include <functional>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/transform.hpp"
#include "thesauros/utility/static-ranges/sinks/reduce.hpp"

namespace thes::star {
struct AllDifferentGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr bool operator()(TRange&& range) const {
    constexpr std::size_t size = star::size<TRange>;
    return star::index_transform<size>([&](auto i) {
             const auto trans = star::index_transform<i + 1, size>(
               [&](auto j) { return get_at(range, i) != get_at(range, j); });
             return trans | star::left_reduce(std::logical_and<>{}, true);
           }) |
           star::left_reduce(std::logical_and<>{}, true);
  }
};

inline constexpr AllDifferentGenerator all_different{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_ALL_DIFFERENT_HPP
