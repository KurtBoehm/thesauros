#include <cstddef>
#include <iostream>
#include <limits>

#include "thesauros/utility.hpp"

#include "tools.hpp"

int main() {
  const std::size_t size = std::numeric_limits<std::size_t>::max() - 10;
  const std::size_t seg_num = 7;

  std::cout << "size: " << size << std::endl;
  std::cout << std::endl;
  {
    const thes::UniformIndexSegmenter<std::size_t> seg(size, seg_num);

    std::size_t start = 0;
    std::size_t sum = 0;
    for (std::size_t i = 0; i < seg_num; ++i) {
      const auto j0 = seg.segment_start(i);
      const auto j1 = seg.segment_end(i);

      std::cout << j0 << "→" << j1 << ":" << (j1 - j0) << std::endl;

      THES_ASSERT(j0 == start);
      start = j1;
      sum += j1 - j0;
    }
    THES_ASSERT(sum == size);
  }
  std::cout << std::endl;
  {
    const std::size_t block_size = 8;
    const thes::BlockedIndexSegmenter<std::size_t> seg(size, seg_num, block_size);

    std::size_t start = 0;
    std::size_t sum = 0;
    for (std::size_t i = 0; i < seg_num; ++i) {
      const auto j0 = seg.segment_start(i);
      const auto j1 = seg.segment_end(i);

      std::cout << j0 << "→" << j1 << ":" << (j1 - j0) << std::endl;

      THES_ASSERT(j0 == start);
      start = j1;
      sum += j1 - j0;
    }
    THES_ASSERT(sum == size);
  }
}
