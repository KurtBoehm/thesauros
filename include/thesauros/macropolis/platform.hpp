#ifndef INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP

namespace thes {
#ifdef __clang__
#define THES_POLIS_CLANG true
#else
#define THES_POLIS_CLANG false
#endif

#ifdef __GNUC__
#define THES_POLIS_GCC_COMPAT true
#else
#define THES_POLIS_GCC_COMPAT false
#endif
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
