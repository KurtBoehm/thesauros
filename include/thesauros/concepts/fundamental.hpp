// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONCEPTS_FUNDAMENTAL_HPP
#define INCLUDE_THESAUROS_CONCEPTS_FUNDAMENTAL_HPP

#include <concepts>

namespace thes {
//--------------------------------------------------------------------------------------------------
// Signed integer types, as defined in [basic.fundamental]/p1
//--------------------------------------------------------------------------------------------------

/** Whether `Type` is one of the standard or extended signed integer types. */
template<typename Type>
concept SignedInteger = std::same_as<Type, signed char> || std::same_as<Type, signed short> ||
                        std::same_as<Type, signed int> || std::same_as<Type, signed long> ||
                        std::same_as<Type, signed long long>
#ifdef __SIZEOF_INT128__
                        || std::same_as<Type, __int128_t>
#endif
  ;

//--------------------------------------------------------------------------------------------------
// Unsigned integer types, as defined in [basic.fundamental]/p2
//--------------------------------------------------------------------------------------------------

/** Whether `Type` is one of the standard or extended unsigned integer types. */
template<typename Type>
concept UnsignedInteger = std::same_as<Type, unsigned char> || std::same_as<Type, unsigned short> ||
                          std::same_as<Type, unsigned int> || std::same_as<Type, unsigned long> ||
                          std::same_as<Type, unsigned long long>
#ifdef __SIZEOF_INT128__
                          || std::same_as<Type, __uint128_t>
#endif
  ;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONCEPTS_FUNDAMENTAL_HPP
