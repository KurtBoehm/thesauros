#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_SIZE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_SIZE_HPP

#include <cstddef>
#include <utility>

namespace thes::star {
namespace impl {
template<typename TRange>
concept HasMemberSize = requires { TRange::size; };

template<typename TRange>
concept HasTupleSize = requires(const TRange& c) { sizeof(std::tuple_size<TRange>); };
} // namespace impl

template<typename TRange>
struct SizeTrait;

template<typename TRange>
requires impl::HasMemberSize<TRange>
struct SizeTrait<TRange> {
  static constexpr std::size_t value = TRange::size;
};
template<typename TRange>
requires(!impl::HasMemberSize<TRange> && impl::HasTupleSize<TRange>)
struct SizeTrait<TRange> {
  static constexpr std::size_t value = std::tuple_size_v<TRange>;
};

template<typename TRange>
requires(requires { sizeof(SizeTrait<TRange>); })
inline constexpr std::size_t size = SizeTrait<TRange>::value;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_SIZE_HPP
