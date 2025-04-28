#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_ALL_DIFFERENT_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_ALL_DIFFERENT_HPP

#include <cstddef>
#include <functional>

#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/ranges/transform.hpp"
#include "thesauros/static-ranges/sinks/reduce.hpp"

namespace thes::star {
struct AllDifferentGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr bool operator()(TRange&& range) const {
    constexpr std::size_t size = star::size<TRange>;
    return star::left_reduce(std::logical_and<>{}, true)(star::index_transform<size>([&](auto i) {
      const auto trans = star::index_transform<i + 1, size>(
        [&](auto j) { return get_at(range, i) != get_at(range, j); });
      return star::left_reduce(std::logical_and<>{}, true)(trans);
    }));
  }
};

inline constexpr AllDifferentGenerator all_different{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_ALL_DIFFERENT_HPP
