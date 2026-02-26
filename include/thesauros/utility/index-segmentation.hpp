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
/**
 * Uniformly partitions contiguous indices into segments.
 *
 * @tparam TSize    Index type.
 * @tparam TSegment Segment index type.
 */
template<typename TSize, typename TSegment>
struct UniformIndexSegmenter {
  using Size = TSize;
  using Segment = TSegment;
  using Shared = Union<Size, Segment>;

  /** Constructs a segmenter for [0, size) into segment_num segments. */
  constexpr UniformIndexSegmenter(Size size, Segment segment_num) noexcept
      : size_{size}, segment_num_{segment_num}, div_{Size(size / segment_num)},
        mod_{Size(size % segment_num)} {}

  /** First index of a segment. */
  [[nodiscard]] constexpr Size segment_start(const Segment segment) const noexcept {
    assert(segment <= segment_num_);
    return Size(segment * div_ + std::min(Shared(mod_), Shared(segment)));
  }

  /** One-past-last index of a segment. */
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const noexcept {
    return segment_start(Segment(segment + 1));
  }

  /** [segment_start, segment_end) as an iota range. */
  [[nodiscard]] constexpr IotaRange<Size> segment_range(const Segment segment) const noexcept {
    return range(segment_start(segment), segment_end(segment));
  }

  /** Segment containing index. */
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

  /** Total number of indices. */
  [[nodiscard]] constexpr Size size() const noexcept {
    return size_;
  }

  /** Number of segments. */
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

/** Uniform segmenter over [offset, offset + size). */
template<typename TSize, typename TSegment>
struct OffsetUniformIndexSegmenter : public UniformIndexSegmenter<TSize, TSegment> {
  using Uniform = UniformIndexSegmenter<TSize, TSegment>;
  using Size = TSize;
  using Segment = TSegment;

  /** Constructs an offset uniform segmenter. */
  constexpr OffsetUniformIndexSegmenter(Size offset, Size size, Segment segment_num) noexcept
      : Uniform{size, segment_num}, offset_{offset} {}

  /** First index of a segment (with offset). */
  [[nodiscard]] constexpr Size segment_start(const Segment segment) const noexcept {
    return Uniform::segment_start(segment) + offset_;
  }

  /** One-past-last index of a segment (with offset). */
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const noexcept {
    return Uniform::segment_end(segment) + offset_;
  }

  /** [segment_start, segment_end) as an iota range. */
  [[nodiscard]] constexpr IotaRange<Size> segment_range(const Segment segment) const noexcept {
    return range(segment_start(segment), segment_end(segment));
  }

  /** Segment containing index, accounting for offset. */
  [[nodiscard]] constexpr Segment segment_of(Size index) const noexcept {
    assert(index >= offset_);
    return Uniform::segment_of(Size(index - offset_));
  }

private:
  Size offset_;
};

/** Uniform segmenter composed with affine transformation index ↦ factor * i + offset. */
template<typename TSize, typename TSegment>
struct AffineUniformIndexSegmenter : public UniformIndexSegmenter<TSize, TSegment> {
  using Uniform = UniformIndexSegmenter<TSize, TSegment>;
  using Size = TSize;
  using Segment = TSegment;

  /** Constructs an affine uniform segmenter. */
  constexpr AffineUniformIndexSegmenter(Size factor, Size offset, Size size,
                                        Segment segment_num) noexcept
      : Uniform{size, segment_num}, factor_{factor}, offset_{offset} {}

  /** First index of a segment after affine transform. */
  [[nodiscard]] constexpr Size segment_start(const Segment segment) const noexcept {
    return factor_ * Uniform::segment_start(segment) + offset_;
  }

  /** One-past-last index of a segment after affine transform. */
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const noexcept {
    return factor_ * Uniform::segment_end(segment) + offset_;
  }

  /** [segment_start, segment_end) as an iota range. */
  [[nodiscard]] constexpr IotaRange<Size> segment_range(const Segment segment) const noexcept {
    return range(segment_start(segment), segment_end(segment));
  }

  /** Segment containing index after inverse affine transform. */
  [[nodiscard]] constexpr Segment segment_of(Size index) const noexcept {
    assert(index >= offset_);
    return Uniform::segment_of(Size((index - offset_) / factor_));
  }

  /** Total number of indices. */
  [[nodiscard]] constexpr Size size() const noexcept {
    return Uniform::size() * factor_;
  }

private:
  Size factor_;
  Size offset_;
};

/**
 * Segments indices into blocks, then uniformly assigns blocks to segments.
 *
 * @tparam TSize    Index and block size type.
 * @tparam TSegment Segment index type.
 */
template<typename TSize, typename TSegment>
struct BlockedIndexSegmenter {
  using Size = TSize;
  using Segment = TSegment;

  /**
   * Constructs a blocked segmenter.
   *
   * @param size        Total number of indices in [0, size).
   * @param segment_num Number of segments.
   * @param block_size  Size of a block in indices.
   */
  constexpr BlockedIndexSegmenter(Size size, Segment segment_num, Size block_size) noexcept
      : size_(size), block_size_(block_size), block_seg_(div_ceil(size, block_size), segment_num) {}

  /** First index of a segment. */
  [[nodiscard]] constexpr Size segment_start(const Segment segment) const noexcept {
    return std::min(saturate_cast<Size>(block_size_ * block_seg_.segment_start(segment)), size_);
  }

  /** One-past-last index of a segment. */
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const noexcept {
    return segment_start(Segment(segment + 1));
  }

  /** [segment_start, segment_end) as an iota range. */
  [[nodiscard]] constexpr auto segment_range(const Segment segment) const noexcept {
    return range(segment_start(segment), segment_end(segment));
  }

  /** Total number of indices. */
  [[nodiscard]] constexpr Size size() const noexcept {
    return size_;
  }

  /** Number of segments. */
  [[nodiscard]] constexpr Segment segment_num() const noexcept {
    return block_seg_.segment_num();
  }

  /** Block size. */
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
