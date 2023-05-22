#include <cstddef>
#include <iostream>
#include <limits>

#include "thesauros/test.hpp"
#include "thesauros/utility.hpp"

int main() {
  const std::size_t size = std::numeric_limits<std::size_t>::max() - 10;
  const std::size_t seg_num = 7;

  std::cout << "size: " << size << '\n';
  std::cout << '\n';
  {
    const thes::UniformIndexSegmenter<std::size_t> seg(size, seg_num);

    std::size_t start = 0;
    std::size_t sum = 0;
    for (std::size_t i = 0; i < seg_num; ++i) {
      const auto j0 = seg.segment_start(i);
      const auto j1 = seg.segment_end(i);

      std::cout << j0 << "→" << j1 << ":" << (j1 - j0) << '\n';

      THES_ASSERT(j0 == start);
      start = j1;
      sum += j1 - j0;
    }
    THES_ASSERT(sum == size);
  }
  std::cout << '\n';
  {
    const std::size_t block_size = 8;
    const thes::BlockedIndexSegmenter<std::size_t> seg(size, seg_num, block_size);

    std::size_t start = 0;
    std::size_t sum = 0;
    for (std::size_t i = 0; i < seg_num; ++i) {
      const auto j0 = seg.segment_start(i);
      const auto j1 = seg.segment_end(i);

      std::cout << j0 << "→" << j1 << ":" << (j1 - j0) << '\n';

      THES_ASSERT(j0 == start);
      start = j1;
      sum += j1 - j0;
    }
    THES_ASSERT(sum == size);
  }
  std::cout << '\n';
  {
    auto impl = []<typename T>(thes::TypeTag<T>) {
      for (const auto num : thes::range<T>(512)) {
        std::cout << "num: " << num << '\n';
        for (const auto blocks : thes::range<T>(1, 256)) {
          // test_to_seg
          {
            const T block_num = num / blocks;
            const thes::UniformIndexSegmenter<T> seg{num, blocks};
            T sum = 0;
            for (const auto i : thes::range(blocks)) {
              const auto i1 = seg.segment_start(i);
              const auto i2 = seg.segment_start(i + 1);
              THES_ASSERT(i1 <= i2);
              const auto block_size = i2 - i1;
              THES_ASSERT(block_size - block_num <= 1);
              sum += block_size;
            }
            THES_ASSERT(sum == num);
          }

          // test_from_seg
          {
            const thes::UniformIndexSegmenter<T> seg{num, blocks};
            for (const auto i : thes::range(num)) {
              const auto s = seg.segment_of(i);
              THES_ASSERT(seg.segment_range(s).contains(i));
            }
          }
        }
      }
    };
    impl(thes::type_tag<std::uint32_t>);
    impl(thes::type_tag<std::uint64_t>);
  }
}
