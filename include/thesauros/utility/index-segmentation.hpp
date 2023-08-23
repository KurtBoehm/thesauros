#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP

#include <algorithm>
#include <cassert>
#include <optional>

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

  UniformIndexSegmenter(Size size, Segment segment_num)
      : size_(size), segment_num_(segment_num), div_(static_cast<Size>(size / segment_num)),
        mod_(static_cast<Size>(size % segment_num)) {}

  [[nodiscard]] Size segment_start(const Segment segment) const {
    assert(segment <= segment_num_);
    return static_cast<Size>(segment * div_ + std::min<Shared>(mod_, segment));
  }
  [[nodiscard]] Size segment_end(const Segment segment) const {
    return segment_start(segment + 1);
  }

  [[nodiscard]] auto segment_range(const Segment segment) const {
    return range(segment_start(segment), segment_start(segment + 1));
  }

  [[nodiscard]] Segment segment_of(Size index) const {
    const Size ref = mod_ * (div_ + 1);
    if (index <= ref) {
      // index / (div + 1)
      return index / div_div1_;
    }
    // mod + (index - ref) / div
    assert(div_div_.has_value());
    return mod_ + (index - ref) / *div_div_;
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
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
