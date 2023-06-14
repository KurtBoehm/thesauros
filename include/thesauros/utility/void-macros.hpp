#ifndef INCLUDE_THESAUROS_UTILITY_VOID_MACROS_HPP
#define INCLUDE_THESAUROS_UTILITY_VOID_MACROS_HPP

namespace thes {
#define THES_APPLY_VALUED_RETURN(TYPE, EXPR) \
  if constexpr (std::is_void_v<TYPE>) { \
    EXPR; \
  } else { \
    if (TYPE value = EXPR; value.has_value()) { \
      return value; \
    } \
  }

#define THES_RETURN_EMPTY_OPTIONAL(TYPE) \
  if constexpr (!std::is_void_v<TYPE>) { \
    return TYPE{}; \
  }
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VOID_MACROS_HPP
