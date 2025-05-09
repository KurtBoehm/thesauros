// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_WARNINGS_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_WARNINGS_HPP

#include "boost/preprocessor.hpp"

#include "thesauros/macropolis/platform.hpp"

#if THES_CLANG
#define THES_POLIS_PRAGMA_CLANG(PRAGMA) _Pragma(PRAGMA)
#else
#define THES_POLIS_PRAGMA_CLANG(PRAGMA)
#endif

#if THES_GCC_COMPAT
#define THES_POLIS_PRAGMA_GCC(PRAGMA) _Pragma(PRAGMA)
#define THES_POLIS_DIAGNOSTICS_IGNORED_OP_GCC(REC, DUMMY, ELEM) \
  THES_POLIS_PRAGMA_GCC(BOOST_PP_STRINGIZE(GCC diagnostic ignored ELEM))

#define THES_POLIS_DIAGNOSTICS_IGNORED_PUSH_gcc(...) \
  _Pragma("GCC diagnostic push") \
    THES_POLIS_PRAGMA_CLANG("clang diagnostic ignored \"-Wunknown-warning-option\"") \
      BOOST_PP_LIST_FOR_EACH(THES_POLIS_DIAGNOSTICS_IGNORED_OP_GCC, BOOST_PP_EMPTY(), \
                             BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))
#define THES_POLIS_DIAGNOSTICS_IGNORED_POP_gcc(...) _Pragma("GCC diagnostic pop")
#else
#define THES_POLIS_DIAGNOSTICS_IGNORED_PUSH_gcc(...)
#define THES_POLIS_DIAGNOSTICS_IGNORED_POP_gcc(...)
#endif

#define THES_POLIS_DIAGNOSTICS_IGNORED_PUSH(COMPILER, ...) \
  THES_POLIS_DIAGNOSTICS_IGNORED_PUSH_##COMPILER(__VA_ARGS__)
#define THES_POLIS_DIAGNOSTICS_IGNORED_POP(COMPILER) THES_POLIS_DIAGNOSTICS_IGNORED_POP_##COMPILER()

#endif // INCLUDE_THESAUROS_MACROPOLIS_WARNINGS_HPP
