#ifndef INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_POSITION_TO_INDEX_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_POSITION_TO_INDEX_HPP

#include <concepts>
#include <cstddef>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/element-type.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TPos, typename TProds>
requires(std::same_as<star::ElementType<TPos>, star::ElementType<TProds>> &&
         star::size<TPos> + 1 == star::size<TProds>)
inline constexpr auto position_to_index(const TPos& pos, const TProds& incl_postfix_products) {
  constexpr auto size = star::size<TPos>;
  return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/) {
    return (... + (star::get_at<tIdxs>(pos) * star::get_at<tIdxs + 1>(incl_postfix_products)));
  }(std::make_index_sequence<size>{});
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_POSITION_TO_INDEX_HPP
