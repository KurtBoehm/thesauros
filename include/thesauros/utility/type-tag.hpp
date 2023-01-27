#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_TAG_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_TAG_HPP

namespace thes {
template<typename T>
struct TypeTag {
  using Type = T;
};

template<typename T>
inline constexpr TypeTag<T> type_tag{};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_TAG_HPP
