// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_VOID_MACROS_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_VOID_MACROS_HPP

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

#endif // INCLUDE_THESAUROS_MACROPOLIS_VOID_MACROS_HPP
