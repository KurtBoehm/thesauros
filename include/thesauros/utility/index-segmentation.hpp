#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP

#include <algorithm>
#include <cassert>
#include <optional>

#include "thesauros/math/divmod.hpp"
#include "thesauros/ranges/iota.hpp"

namespace thes {
template<typename TIndex>
struct UniformIndexSegmenter {
  using Index = TIndex;

  UniformIndexSegmenter(Index size, Index segment_num)
      : size_(size), segment_num_(segment_num), div_(size / segment_num), mod_(size % segment_num) {
  }

  [[nodiscard]] Index segment_start(const Index segment) const {
    return segment * div_ + std::min(mod_, segment);
  }
  [[nodiscard]] Index segment_end(const Index segment) const {
    return segment_start(segment + 1);
  }

  [[nodiscard]] auto segment_range(const Index segment) const {
    return range(segment_start(segment), segment_start(segment + 1));
  }

  [[nodiscard]] Index segment_of(Index index) const {
    const Index ref = mod_ * (div_ + 1);
    if (index <= ref) {
      // index / (div + 1)
      return index / div_div1_;
    }
    // mod + (index - ref) / div
    assert(div_div_.has_value());
    return mod_ + (index - ref) / *div_div_;
  }

private:
  using Div = Divisor<Index>;
  using OptDiv = std::optional<Div>;

  Index size_;
  Index segment_num_;
  Index div_;
  Index mod_;
  OptDiv div_div_{div_ == 0 ? OptDiv{} : Div{div_}};
  Div div_div1_{div_ + 1};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
