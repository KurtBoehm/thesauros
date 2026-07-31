// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MATH_INTEGER_CAST_HPP
#define INCLUDE_THESAUROS_MATH_INTEGER_CAST_HPP

#include <concepts>
#include <limits>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/utility/info-result.hpp"

namespace thes {
enum struct CastInfo : u8 { okay, too_small, too_large };
template<typename T>
using CastResult = InfoResult<T, CastInfo, CastInfo::okay>;

namespace detail {
template<std::integral Dst>
struct SafeCastWorker {
  using Ret = CastResult<Dst>;

  THES_ALWAYS_INLINE explicit constexpr SafeCastWorker(Dst v) : value_(v) {}

  THES_ALWAYS_INLINE constexpr void too_small() {
    info_ = CastInfo::too_small;
  }
  THES_ALWAYS_INLINE constexpr void too_large() {
    info_ = CastInfo::too_large;
  }
  THES_ALWAYS_INLINE constexpr Ret value() {
    return Ret{value_, info_};
  }

private:
  Dst value_;
  CastInfo info_{CastInfo::okay};
};
template<std::integral Dst>
struct SatCastWorker {
  using Ret = Dst;

  THES_ALWAYS_INLINE explicit constexpr SatCastWorker(Dst v) : value_(v) {}

  THES_ALWAYS_INLINE constexpr void too_small() {
    value_ = std::numeric_limits<Dst>::lowest();
  }
  THES_ALWAYS_INLINE constexpr void too_large() {
    value_ = std::numeric_limits<Dst>::max();
  }
  THES_ALWAYS_INLINE constexpr Ret value() {
    return value_;
  }

private:
  Dst value_;
};
} // namespace detail

template<typename Src, typename Dst, typename Worker>
struct CastTrait;
template<typename Src, std::unsigned_integral Dst, typename Worker>
struct CastTrait<Src, Dst, Worker> {
  using Ret = Worker::Ret;

  static constexpr Ret cast(Src in) {
    Worker out{static_cast<Dst>(in)};
    if constexpr (std::signed_integral<Src>) {
      if (in < 0) {
        out.too_small();
      }
    }
    if constexpr (std::numeric_limits<Dst>::digits < std::numeric_limits<Src>::digits) {
      if (in > Src{std::numeric_limits<Dst>::max()}) {
        out.too_large();
      }
    }
    return out.value();
  }
};
template<typename Src, std::signed_integral Dst, typename Worker>
struct CastTrait<Src, Dst, Worker> {
  using Ret = Worker::Ret;

  static constexpr Ret cast(Src in) {
    Worker out{static_cast<Dst>(in)};
    if constexpr (std::numeric_limits<Dst>::digits < std::numeric_limits<Src>::digits) {
      if constexpr (std::signed_integral<Src>) {
        if (in < Src{std::numeric_limits<Dst>::lowest()}) {
          out.too_small();
        }
      }
      if (in > Src{std::numeric_limits<Dst>::max()}) {
        out.too_large();
      }
    }
    return out.value();
  }
};

template<std::integral Dst, std::integral Src>
constexpr CastResult<Dst> safe_cast(Src in) {
  return CastTrait<Src, Dst, detail::SafeCastWorker<Dst>>::cast(in);
}
template<std::integral Dst, std::integral Src>
constexpr Dst saturate_cast(Src in) {
  return CastTrait<Src, Dst, detail::SatCastWorker<Dst>>::cast(in);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_INTEGER_CAST_HPP
