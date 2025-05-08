// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_PRINTABLE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_PRINTABLE_HPP

#include <concepts>
#include <type_traits>

namespace thes::star {
struct PrintableMarker {};
template<typename T>
struct Printable : public std::false_type {};
template<typename T>
requires(std::same_as<std::decay_t<decltype(T::printable)>, PrintableMarker>)
struct Printable<T> : public std::true_type {};

template<typename T>
concept AnyPrintable = Printable<T>::value;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_PRINTABLE_HPP
