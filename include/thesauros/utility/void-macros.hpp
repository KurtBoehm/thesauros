#ifndef INCLUDE_THESAUROS_UTILITY_VOID_MACROS_HPP
#define INCLUDE_THESAUROS_UTILITY_VOID_MACROS_HPP

#include <boost/preprocessor.hpp>

namespace thes {
#define THES_APPLY_VALUED_RETURN_EX(TYPE, EXPR, AFTER) \
  if constexpr (std::is_void_v<TYPE>) { \
    EXPR; \
    BOOST_PP_REMOVE_PARENS(AFTER) \
  } else { \
    TYPE value = EXPR; \
    BOOST_PP_REMOVE_PARENS(AFTER) \
    if (value.has_value()) { \
      return value; \
    } \
  }

#define THES_APPLY_VALUED_RETURN(TYPE, EXPR) THES_APPLY_VALUED_RETURN_EX(TYPE, EXPR, ())

#define THES_RETURN_EMPTY_OPTIONAL(TYPE) \
  if constexpr (!std::is_void_v<TYPE>) { \
    return TYPE{}; \
  }
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VOID_MACROS_HPP
