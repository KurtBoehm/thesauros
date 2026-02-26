// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP

#include <algorithm>
#include <cassert>
#include <optional>

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/divmod.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/types/type-transformations.hpp"

namespace thes {
template<typename TSize, typename TSegment>
struct UniformIndexSegmenter {
  using Size = TSize;
  using Segment = TSegment;
  using Shared = Union<Size, Segment>;

  constexpr UniformIndexSegmenter(Size size, Segment segment_num) noexcept
      : size_{size}, segment_num_{segment_num}, div_{Size(size / segment_num)},
        mod_{Size(size % segment_num)} {}

  [[nodiscard]] constexpr Size segment_start(const Segment segment) const noexcept {
    assert(segment <= segment_num_);
    return static_cast<Size>(segment * div_ + std::min<Shared>(mod_, segment));
  }
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const noexcept {
    return segment_start(Segment(segment + 1));
  }

  [[nodiscard]] constexpr IotaRange<Size> segment_range(const Segment segment) const noexcept {
    return range(segment_start(segment), segment_end(segment));
  }

  [[nodiscard]] constexpr Segment segment_of(Size index) const noexcept {
    const Size ref = Size(mod_ * (div_ + 1));
    if (index <= ref) {
      // index / (div + 1)
      return Segment(index / div_div1_);
    }
    // mod + (index - ref) / div
    assert(div_div_.has_value());
    return Segment(mod_ + (index - ref) / *div_div_);
  }

  [[nodiscard]] constexpr Size size() const noexcept {
    return size_;
  }
  [[nodiscard]] constexpr Segment segment_num() const noexcept {
    return segment_num_;
  }

private:
  using Div = Divisor<Size>;
  using OptDiv = std::optional<Div>;

  Size size_;
  Segment segment_num_;
  Size div_;
  Size mod_;
  OptDiv div_div_{div_ == 0 ? OptDiv{} : Div{div_}};
  Div div_div1_{*safe_cast<Size>(div_ + 1)};
};

template<typename TSize, typename TSegment>
struct OffsetUniformIndexSegmenter : public UniformIndexSegmenter<TSize, TSegment> {
  using Uniform = UniformIndexSegmenter<TSize, TSegment>;
  using Size = TSize;
  using Segment = TSegment;

  constexpr OffsetUniformIndexSegmenter(Size offset, Size size, Segment segment_num) noexcept
      : Uniform{size, segment_num}, offset_{offset} {}

  [[nodiscard]] constexpr Size segment_start(const Segment segment) const noexcept {
    return Uniform::segment_start(segment) + offset_;
  }
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const noexcept {
    return Uniform::segment_end(segment) + offset_;
  }

  [[nodiscard]] constexpr IotaRange<Size> segment_range(const Segment segment) const noexcept {
    return range(segment_start(segment), segment_end(segment));
  }

  [[nodiscard]] constexpr Segment segment_of(Size index) const noexcept {
    assert(index >= offset_);
    return Uniform::segment_of(Size(index - offset_));
  }

private:
  Size offset_;
};

template<typename TSize, typename TSegment>
struct BlockedIndexSegmenter {
  using Size = TSize;
  using Segment = TSegment;

  constexpr BlockedIndexSegmenter(Size size, Segment segment_num, Size block_size) noexcept
      : size_(size), block_size_(block_size), block_seg_(div_ceil(size, block_size), segment_num) {}

  [[nodiscard]] constexpr Size segment_start(const Segment segment) const noexcept {
    return std::min(saturate_cast<Size>(block_size_ * block_seg_.segment_start(segment)), size_);
  }
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const noexcept {
    return segment_start(Segment(segment + 1));
  }
  [[nodiscard]] constexpr auto segment_range(const Segment segment) const noexcept {
    return range(segment_start(segment), segment_end(segment));
  }

  [[nodiscard]] constexpr Size size() const noexcept {
    return size_;
  }
  [[nodiscard]] constexpr Segment segment_num() const noexcept {
    return block_seg_.segment_num();
  }
  [[nodiscard]] constexpr Size block_size() const noexcept {
    return block_size_;
  }

private:
  Size size_;
  Size block_size_;
  UniformIndexSegmenter<TSize, TSegment> block_seg_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
