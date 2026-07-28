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
namespace detail::generate {
template<typename Ret, typename Fun>
struct ValueBase {};

template<typename Ret, typename Fun>
requires(!std::is_void_v<Ret>)
struct ValueBase<Ret, Fun> {
  using Value = Ret;
};
} // namespace detail::generate

template<std::size_t Size, typename Ret, std::invocable<> Gen>
struct Generate : public detail::generate::ValueBase<Ret, Gen> {
  static constexpr std::size_t size = Size;
  static constexpr TupleDefsMarker tuple_defs_marker{};

  explicit constexpr Generate(Gen&& gen) : gen_{std::forward<Gen>(gen)} {}

  template<std::size_t I>
  requires(I < Size)
  THES_ALWAYS_INLINE friend constexpr auto get(const Generate& self) {
    return self.gen_();
  }

private:
  Gen gen_;
};

template<std::size_t Size, std::invocable<> Gen>
THES_ALWAYS_INLINE inline constexpr auto generate(Gen&& gen) {
  return Generate<Size, void, Gen>{std::forward<Gen>(gen)};
}
template<typename Ret, std::size_t Size, std::invocable<> Gen>
THES_ALWAYS_INLINE inline constexpr auto generate(Gen&& gen) {
  return Generate<Size, Ret, Gen>{std::forward<Gen>(gen)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_GENERATE_HPP
