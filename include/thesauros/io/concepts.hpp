#ifndef INCLUDE_THESAUROS_IO_CONCEPTS_HPP
#define INCLUDE_THESAUROS_IO_CONCEPTS_HPP

#include <ostream>
#include <type_traits>

namespace thes {
template<typename T>
concept StdOStream = std::is_base_of_v<std::ostream, T>;

template<typename T>
struct IsOutStreamTrait : std::is_base_of<std::ostream, T> {};

template<typename T>
concept OutStream = IsOutStreamTrait<T>::value;
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_CONCEPTS_HPP
