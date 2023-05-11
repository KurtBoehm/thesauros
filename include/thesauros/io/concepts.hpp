#ifndef INCLUDE_THESAUROS_IO_CONCEPTS_HPP
#define INCLUDE_THESAUROS_IO_CONCEPTS_HPP

#include <ostream>
#include <type_traits>

namespace thes {
template<typename T>
concept OStream = std::is_base_of_v<std::ostream, T>;
}

#endif // INCLUDE_THESAUROS_IO_CONCEPTS_HPP
