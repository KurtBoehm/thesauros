#ifndef INCLUDE_THESAUROS_STRING_CHARACTER_TOOLS_HPP
#define INCLUDE_THESAUROS_STRING_CHARACTER_TOOLS_HPP

namespace thes {
inline constexpr bool is_uppercase(char c) {
  return 'A' <= c && c <= 'Z';
}
inline constexpr char to_lowercase(char c) {
  if (is_uppercase(c)) {
    c -= 'A';
    c += 'a';
  }
  return c;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_STRING_CHARACTER_TOOLS_HPP
