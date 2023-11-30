#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_NAME_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_NAME_HPP

#include <cstdlib>
#include <memory>
#include <string>

#include <cxxabi.h>

namespace thes {
template<typename T>
std::string type_name() {
  using CSmartPtr = std::unique_ptr<char[], decltype([](auto* p) { std::free(p); })>;

  int status{};
  CSmartPtr ptr{abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status)};
  if (status != 0) {
    return std::string(typeid(T).name());
  }
  return std::string(ptr.get());
}

template<typename T>
std::string type_name(T&& /*value*/) {
  return type_name<T>();
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_NAME_HPP
