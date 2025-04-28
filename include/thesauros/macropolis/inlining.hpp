#ifndef INCLUDE_THESAUROS_MACROPOLIS_INLINING_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_INLINING_HPP

namespace thes {
#define THES_ALWAYS_INLINE __attribute__((always_inline))
#define THES_NEVER_INLINE __attribute__((noinline))
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_INLINING_HPP
