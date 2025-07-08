// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_GENERATE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_GENERATE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"

namespace thes::star {
namespace generate_impl {
template<typename TRet, typename TFun>
struct ValueBase {};

template<typename TRet, typename TFun>
requires(!std::is_void_v<TRet>)
struct ValueBase<TRet, TFun> {
  using Value = TRet;
};
} // namespace generate_impl

template<std::size_t tSize, typename TRet, std::invocable<> TGen>
struct Generate : public generate_impl::ValueBase<TRet, TGen> {
  static constexpr std::size_t size = tSize;
  static constexpr TupleDefsMarker tuple_defs_marker{};

  explicit constexpr Generate(TGen&& gen) : gen_{std::forward<TGen>(gen)} {}

  template<std::size_t tIndex>
  requires(tIndex < tSize)
  THES_ALWAYS_INLINE friend constexpr auto get(const Generate& self) {
    return self.gen_();
  }

private:
  TGen gen_;
};

template<std::size_t tSize, std::invocable<> TGen>
THES_ALWAYS_INLINE inline constexpr auto generate(TGen&& gen) {
  return Generate<tSize, void, TGen>{std::forward<TGen>(gen)};
}
template<typename TRet, std::size_t tSize, std::invocable<> TGen>
THES_ALWAYS_INLINE inline constexpr auto generate(TGen&& gen) {
  return Generate<tSize, TRet, TGen>{std::forward<TGen>(gen)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_GENERATE_HPP
