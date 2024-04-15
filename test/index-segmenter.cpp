#include <cstddef>
#include <cstdint>
#include <limits>

#include "thesauros/format.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"
#include "thesauros/utility.hpp"

int main() {
  const std::size_t size = std::numeric_limits<std::size_t>::max() - 10;
  const std::size_t seg_num = 7;

  fmt::print("size: {}\n\n", size);

  {
    const thes::UniformIndexSegmenter seg(size, seg_num);

    std::size_t start = 0;
    std::size_t sum = 0;
    for (std::size_t i = 0; i < seg_num; ++i) {
      const auto j0 = seg.segment_start(i);
      const auto j1 = seg.segment_end(i);

      fmt::print("{}→{}:{}\n", j0, j1, j1 - j0);

      THES_ASSERT(j0 == start);
      start = j1;
      sum += j1 - j0;
    }
    THES_ASSERT(sum == size);
  }
  fmt::print("\n");
  {
    auto impl = []<typename T>(thes::TypeTag<T>) {
      for (const auto num : thes::range<T>(512)) {
        fmt::print("num: {}\n", num);
        for (const auto blocks : thes::range<T>(1, 256)) {
          // test_to_seg
          {
            const T block_num = num / blocks;
            const thes::UniformIndexSegmenter seg{num, blocks};
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
            const thes::UniformIndexSegmenter seg{num, blocks};
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

  {
    using I = unsigned long;
    constexpr thes::BlockedIndexSegmenter<I, I> seg{362, 8, 4};

    static_assert(seg.segment_range(0) == thes::range<I>(0, 48));
    static_assert(seg.segment_range(1) == thes::range<I>(48, 96));
    static_assert(seg.segment_range(2) == thes::range<I>(96, 144));
    static_assert(seg.segment_range(3) == thes::range<I>(144, 188));
    static_assert(seg.segment_range(4) == thes::range<I>(188, 232));
    static_assert(seg.segment_range(5) == thes::range<I>(232, 276));
    static_assert(seg.segment_range(6) == thes::range<I>(276, 320));
    static_assert(seg.segment_range(7) == thes::range<I>(320, 362));
  }
}
