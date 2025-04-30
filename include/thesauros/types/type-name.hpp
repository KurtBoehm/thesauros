// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_TYPE_NAME_HPP
#define INCLUDE_THESAUROS_TYPES_TYPE_NAME_HPP

#include <cstdlib>
#include <memory>
#include <string>
#include <typeindex>

#include <cxxabi.h>

namespace thes {
inline std::string demangle(const char* name) {
  using CSmartPtr = std::unique_ptr<char[], decltype([](auto* p) { std::free(p); })>;

  int status{};
  CSmartPtr ptr{abi::__cxa_demangle(name, nullptr, nullptr, &status)};
  if (status != 0) {
    return name;
  }
  return ptr.get();
}
inline std::string demangle(std::type_index ti) {
  return demangle(ti.name());
}

template<typename T>
inline std::string type_name() {
  return demangle(typeid(T).name());
}
template<typename T>
inline std::string type_name(T&& /*value*/) {
  return demangle(typeid(T).name());
}
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_TYPE_NAME_HPP
