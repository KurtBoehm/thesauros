#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP

#include <algorithm>
#include <cassert>
#include <optional>

#include "thesauros/math/arithmetic.hpp"
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

template<typename TIndex>
struct BlockedIndexSegmenter {
  using Index = TIndex;

  BlockedIndexSegmenter(Index size, Index segment_num, Index block_size)
      : size_(size), segment_num_(segment_num), block_size_(block_size) {}

  [[nodiscard]] Index segment_start(const Index segment) const {
    return block_size_ * prod_div(block_num_, segment, segment_num_);
  }
  [[nodiscard]] Index segment_end(const Index segment) const {
    return std::min(segment_start(segment + 1), size_);
  }

private:
  Index size_;
  Index segment_num_;
  Index block_size_;
  Index block_num_{div_ceil(size_, block_size_)};
};

template<typename TIndex>
struct UniformIndexSegmenterWithBlock : public UniformIndexSegmenter<TIndex> {
  using Index = TIndex;

  UniformIndexSegmenterWithBlock(Index size, Index segment_num, Index block_size)
      : UniformIndexSegmenter<Index>(size, segment_num), block_size_{block_size} {}

  [[nodiscard]] Index block_size() const {
    return block_size_;
  }

private:
  Index block_size_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_SEGMENTATION_HPP
