#ifndef INCLUDE_THESAUROS_UTILITY_UNREACHABLE_HPP
#define INCLUDE_THESAUROS_UTILITY_UNREACHABLE_HPP

namespace thes {
[[noreturn]] inline void unreachable() {
  // Based on https://en.cppreference.com/w/cpp/utility/unreachable
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  __assume(false);
#else // GCC, Clang
  __builtin_unreachable();
#endif
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_UNREACHABLE_HPP
