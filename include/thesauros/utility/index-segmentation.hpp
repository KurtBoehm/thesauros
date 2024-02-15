#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP

#include <algorithm>
#include <cassert>
#include <optional>

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/divmod.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/safe-cast.hpp"
#include "thesauros/utility/type-transformations.hpp"

namespace thes {
template<typename TSize, typename TSegment>
struct UniformIndexSegmenter {
  using Size = TSize;
  using Segment = TSegment;
  using Shared = Union<Size, Segment>;

  constexpr UniformIndexSegmenter(Size size, Segment segment_num)
      : size_(size), segment_num_(segment_num), div_(static_cast<Size>(size / segment_num)),
        mod_(static_cast<Size>(size % segment_num)) {}

  [[nodiscard]] constexpr Size segment_start(const Segment segment) const {
    assert(segment <= segment_num_);
    return static_cast<Size>(segment * div_ + std::min<Shared>(mod_, segment));
  }
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const {
    return segment_start(segment + 1);
  }

  [[nodiscard]] constexpr auto segment_range(const Segment segment) const {
    return range(segment_start(segment), segment_end(segment));
  }

  [[nodiscard]] constexpr Segment segment_of(Size index) const {
    const Size ref = mod_ * (div_ + 1);
    if (index <= ref) {
      // index / (div + 1)
      return index / div_div1_;
    }
    // mod + (index - ref) / div
    assert(div_div_.has_value());
    return mod_ + (index - ref) / *div_div_;
  }

  [[nodiscard]] constexpr Size size() const {
    return size_;
  }
  [[nodiscard]] constexpr Segment segment_num() const {
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
struct BlockedIndexSegmenter {
  using Size = TSize;
  using Segment = TSegment;

  constexpr BlockedIndexSegmenter(Size size, Segment segment_num, Size block_size)
      : size_(size), block_size_(block_size),
        block_seg_(thes::div_ceil(size, block_size), segment_num) {}

  [[nodiscard]] constexpr Size segment_start(const Segment segment) const {
    return std::min(block_size_ * block_seg_.segment_start(segment), size_);
  }
  [[nodiscard]] constexpr Size segment_end(const Segment segment) const {
    return segment_start(segment + 1);
  }
  [[nodiscard]] constexpr auto segment_range(const Segment segment) const {
    return range(segment_start(segment), segment_end(segment));
  }

  [[nodiscard]] constexpr Size size() const {
    return size_;
  }
  [[nodiscard]] constexpr Segment segment_num() const {
    return block_seg_.segment_num();
  }
  [[nodiscard]] constexpr Size block_size() const {
    return block_size_;
  }

private:
  Size size_;
  Size block_size_;
  UniformIndexSegmenter<TSize, TSegment> block_seg_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
