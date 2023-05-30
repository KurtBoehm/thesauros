#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_TRAITS_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_TRAITS_HPP

#include <type_traits>
namespace thes {
template<typename T>
concept ConstAccess = std::is_const_v<std::remove_reference_t<T>>;
}

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_TRAITS_HPP
