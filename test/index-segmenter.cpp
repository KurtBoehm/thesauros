// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>

#include "thesauros/format.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"
#include "thesauros/types.hpp"
#include "thesauros/utility.hpp"

using thes::range;

namespace {
template<thes::IndexSegmenter TSeg>
void check_segmenter_partition(const TSeg& seg) {
  using Size = TSeg::Size;
  using Segment = TSeg::Segment;

  const Size total = seg.size();
  const Segment num_seg = seg.segment_num();

  // Check that segments form a contiguous, non-overlapping partition of [0, size).
  Size last_end = 0;
  Size sum = 0;

  for (Segment s = 0; s < num_seg; ++s) {
    const Size start = seg.segment_start(s);
    const Size end = seg.segment_end(s);

    // Basic invariants
    THES_ALWAYS_ASSERT(start <= end);
    THES_ALWAYS_ASSERT(start == last_end); // contiguous
    last_end = end;
    sum += end - start;

    // Range object matches start/end
    const auto r = seg.segment_range(s);
    THES_ALWAYS_ASSERT(r.begin_value() == start);
    THES_ALWAYS_ASSERT(r.end_value() == end);

    // segment_of(i) should return s for all i in this segment
    for (Size i : range(start, end)) {
      const Segment s2 = seg.segment_of(i);
      THES_ALWAYS_ASSERT(s2 == s);
      THES_ALWAYS_ASSERT(seg.segment_range(s2).contains(i));
    }
  }

  THES_ALWAYS_ASSERT(last_end == total);
  THES_ALWAYS_ASSERT(sum == total);

  // Check that segment_of() never goes out of range for all indices
  for (Size i : range(total)) {
    const Segment s = seg.segment_of(i);
    THES_ALWAYS_ASSERT(s < num_seg);
    THES_ALWAYS_ASSERT(seg.segment_range(s).contains(i));
  }
}
} // namespace

int main() {
  {
    const std::size_t size = std::numeric_limits<std::size_t>::max() - 10;
    const std::size_t seg_num = 7;

    fmt::print("size: {}\n\n", size);

    const thes::UniformIndexSegmenter seg(size, seg_num);

    // Contiguous coverage and sum check
    std::size_t start = 0;
    std::size_t sum = 0;
    for (std::size_t i = 0; i < seg_num; ++i) {
      const auto j0 = seg.segment_start(i);
      const auto j1 = seg.segment_end(i);

      fmt::print("{}→{}:{}\n", j0, j1, j1 - j0);

      THES_ALWAYS_ASSERT(j0 == start);
      start = j1;
      sum += j1 - j0;
    }
    THES_ALWAYS_ASSERT(sum == size);

    THES_ALWAYS_ASSERT(seg.segment_start(0) == 0);
    THES_ALWAYS_ASSERT(seg.segment_end(seg_num - 1) == size);
  }
  fmt::print("\n");

  {
    auto impl = []<typename T>(thes::TypeTag<T>) {
      for (const auto num : range<T>(512)) {
        fmt::print("UniformIndexSegmenter<T={}>, num: {}\n", thes::type_name<T>(), num);
        for (const auto blocks : range<T>(1, 256)) {
          // segment sizes
          {
            const T block_num = num / blocks;
            const thes::UniformIndexSegmenter seg{num, blocks};
            T sum = 0;
            for (const auto i : range(blocks)) {
              const auto i1 = seg.segment_start(i);
              const auto i2 = seg.segment_start(i + 1);
              THES_ALWAYS_ASSERT(i1 <= i2);
              const auto block_size = i2 - i1;
              // each block differs by at most 1 from the floor
              THES_ALWAYS_ASSERT(block_size == block_num || block_size == block_num + 1 ||
                                 block_num == 0);
              sum += block_size;

              // ranges must be non-empty except possibly when num < blocks
              if (num >= blocks) {
                THES_ALWAYS_ASSERT(block_size > 0);
              }
            }
            THES_ALWAYS_ASSERT(sum == num);
            THES_ALWAYS_ASSERT(seg.size() == num);
            THES_ALWAYS_ASSERT(seg.segment_num() == blocks);
          }

          // segment_of coverage
          {
            const thes::UniformIndexSegmenter seg{num, blocks};
            for (const auto i : range(num)) {
              const auto s = seg.segment_of(i);
              THES_ALWAYS_ASSERT(s < blocks);
              THES_ALWAYS_ASSERT(seg.segment_range(s).contains(i));
            }
          }
        }
      }
    };
    impl(thes::type_tag<std::uint32_t>);
    impl(thes::type_tag<std::uint64_t>);
  }

  {
    using Size = thes::u32;
    using Segment = thes::u16;

    for (Size size : {Size(0), Size(1), Size(2), Size(10), Size(100)}) {
      for (Segment seg_num : range<Segment>(1, 8)) {
        for (Size offset : {Size(0), Size(1), Size(5), Size(1000)}) {
          thes::OffsetUniformIndexSegmenter seg{offset, size, seg_num};

          // Basic invariants: total size and num segments
          THES_ALWAYS_ASSERT(seg.size() == size);
          THES_ALWAYS_ASSERT(seg.segment_num() == seg_num);

          // Contiguous coverage of [offset, offset + size)
          Size last_end = offset;
          Size sum = 0;
          for (Segment s : range(seg_num)) {
            const Size start = seg.segment_start(s);
            const Size end = seg.segment_end(s);
            THES_ALWAYS_ASSERT(start <= end);
            THES_ALWAYS_ASSERT(start == last_end);
            last_end = end;
            sum += end - start;

            // Check that segment_of() in mapped domain is correct
            for (Size i : range(start, end)) {
              const Segment s2 = seg.segment_of(i);
              THES_ALWAYS_ASSERT(s2 == s);
              THES_ALWAYS_ASSERT(seg.segment_range(s2).contains(i));
            }
          }
          THES_ALWAYS_ASSERT(last_end == offset + size);
          THES_ALWAYS_ASSERT(sum == size);

          // segment_of should hit the right segment for all indices in [offset, offset + size)
          for (Size i : range(offset, offset + size)) {
            const Segment s = seg.segment_of(i);
            THES_ALWAYS_ASSERT(seg.segment_range(s).contains(i));
          }
        }
      }
    }
  }

  {
    using Size = std::uint32_t;
    using Segment = std::uint16_t;

    for (Size base_size : {Size(0), Size(1), Size(2), Size(7), Size(23)}) {
      for (Segment seg_num : range<Segment>(1, 8)) {
        for (Size factor : {Size(1), Size(2), Size(3), Size(5)}) {
          for (Size offset : {Size(0), Size(1), Size(10)}) {
            thes::AffineUniformIndexSegmenter seg{factor, offset, base_size, seg_num};

            // Total size must be factor * base_size
            THES_ALWAYS_ASSERT(seg.size() == base_size * factor);
            THES_ALWAYS_ASSERT(seg.segment_num() == seg_num);

            // Check contiguous coverage of [offset, offset + factor * base_size)
            Size last_end = offset;
            Size sum = 0;
            for (Segment s : range(seg_num)) {
              const Size start = seg.segment_start(s);
              const Size end = seg.segment_end(s);
              THES_ALWAYS_ASSERT(start <= end);
              THES_ALWAYS_ASSERT(start == last_end);
              last_end = end;
              sum += end - start;

              // all indices in [start, end) must map back to this segment
              for (Size i : range(start, end)) {
                const Segment s2 = seg.segment_of(i);
                THES_ALWAYS_ASSERT(s2 == s);
                THES_ALWAYS_ASSERT(seg.segment_range(s2).contains(i));
              }
            }
            THES_ALWAYS_ASSERT(last_end == offset + factor * base_size);
            THES_ALWAYS_ASSERT(sum == factor * base_size);

            for (Size i : range(offset, offset + factor * base_size)) {
              const Segment s = seg.segment_of(i);
              THES_ALWAYS_ASSERT(seg.segment_range(s).contains(i));
            }
          }
        }
      }
    }
  }

  {
    using I = unsigned long;
    constexpr thes::BlockedIndexSegmenter<I, I> seg{362, 8, 4};

    static_assert(seg.segment_range(0) == range<I>(0, 48));
    static_assert(seg.segment_range(1) == range<I>(48, 96));
    static_assert(seg.segment_range(2) == range<I>(96, 144));
    static_assert(seg.segment_range(3) == range<I>(144, 188));
    static_assert(seg.segment_range(4) == range<I>(188, 232));
    static_assert(seg.segment_range(5) == range<I>(232, 276));
    static_assert(seg.segment_range(6) == range<I>(276, 320));
    static_assert(seg.segment_range(7) == range<I>(320, 362));
  }

  {
    using Size = std::uint32_t;
    using Segment = std::uint16_t;

    for (Size size :
         {Size(0), Size(1), Size(2), Size(15), Size(63), Size(64), Size(65), Size(127)}) {
      for (Segment seg_num : range<Segment>(1, 9)) {
        for (Size block_size : {Size(1), Size(2), Size(3), Size(4), Size(16)}) {
          thes::BlockedIndexSegmenter<Size, Segment> seg{size, seg_num, block_size};

          THES_ALWAYS_ASSERT(seg.size() == size);
          THES_ALWAYS_ASSERT(seg.segment_num() == seg_num);
          THES_ALWAYS_ASSERT(seg.block_size() == block_size);

          // Basic partition test
          check_segmenter_partition(seg);
        }
      }
    }
  }

  {
    using Size = std::uint32_t;
    using Segment = std::uint16_t;

    auto test_case = [](Size size, Segment seg_num, Size n0, Size n1) {
      thes::UniformIndexSegmenter<Size, Segment> base{size, seg_num};
      thes::PaddedIndexSegmenter padded{base, n0, n1};

      // Global expected range is [0, size + n0 + n1)
      THES_ALWAYS_ASSERT(padded.size() == size + n0 + n1);
      THES_ALWAYS_ASSERT(padded.segment_num() == seg_num);

      // First and last segments must reflect padding correctly
      if (seg_num > 0) {
        THES_ALWAYS_ASSERT(padded.segment_start(0) == 0);
        THES_ALWAYS_ASSERT(padded.segment_end(0) >= n0); // at least includes the leading pad

        const Segment last = seg_num - 1;
        THES_ALWAYS_ASSERT(padded.segment_end(last) == base.segment_end(last) + n0 + n1);
      }

      // Generic partition test (contiguous coverage, segment_of matches)
      check_segmenter_partition(padded);

      // Check that the shifted area [n0, n0 + size) corresponds to base
      for (Size i : range(size)) {
        const Size global_i = i + n0;
        const Segment s_p = padded.segment_of(global_i);
        const Segment s_b = base.segment_of(i);

        // padded and base must agree on segment indices in the shifted region
        THES_ALWAYS_ASSERT(s_p == s_b);

        const auto r_p = padded.segment_range(s_p);
        const auto r_b = base.segment_range(s_b);

        // ranges should be shifted by n0 in this “inner” region,
        // though note padding may extend first/last segments beyond this area.
        THES_ALWAYS_ASSERT(r_p.contains(global_i));
        THES_ALWAYS_ASSERT(r_b.contains(i));
      }

      // Leading padding [0, n0) belongs entirely to segment 0 if seg_num > 0
      if (seg_num > 0 && n0 > 0) {
        for (Size i : range<Size>(n0)) {
          THES_ALWAYS_ASSERT(padded.segment_of(i) == Segment(0));
        }
      }

      // Trailing padding [n0 + size, n0 + size + n1) belongs to last segment
      if (seg_num > 0 && n1 > 0) {
        const Segment last = seg_num - 1;
        for (Size i : range<Size>(n0 + size, n0 + size + n1)) {
          THES_ALWAYS_ASSERT(padded.segment_of(i) == last);
        }
      }
    };

    // Small sizes and various paddings
    for (Size size : {Size(0), Size(1), Size(2), Size(5), Size(16)}) {
      for (Segment seg_num : range<Segment>(1, 6)) {
        for (Size n0 : {Size(0), Size(1), Size(3)}) {
          for (Size n1 : {Size(0), Size(2), Size(4)}) {
            test_case(size, seg_num, n0, n1);
          }
        }
      }
    }

    // A larger example
    test_case(Size(1000), Segment(7), Size(5), Size(13));
  }
}
