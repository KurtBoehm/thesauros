// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>

#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/value-tag.hpp"
#include "thesauros/utility/multi-size.hpp"

namespace test = thes::test;

namespace {
using Size = thes::u32;
using Axes = std::array<Size, 3>;
using Basic = thes::BasicMultiSize<Size, 3>;
using Multi = thes::MultiSize<Size, 3>;
using Sub2 = thes::SubMultiSize<Size, 2>;
using Sub3 = thes::SubMultiSize<Size, 3>;

static_assert(Basic::dimension_num == 3);
static_assert(Multi::dimension_num == 3);

/** A 2×3×4 index space, i.e. 24 elements in row-major order. */
const Axes sizes{2, 3, 4};

//==================================================================================================
// BasicMultiSize
//==================================================================================================

/** Checks the axis sizes and the inclusive postfix products they induce. */
THES_TEST_CASE("axis sizes and postfix products", "[utility][multi-size]") {
  const Basic ms{sizes};

  THES_CHECK(test::range_eq(ms.sizes(), sizes));
  THES_CHECK(ms.total_size() == 24);

  THES_CHECK(ms.axis_size(0) == 2);
  THES_CHECK(ms.axis_size(1) == 3);
  THES_CHECK(ms.axis_size(2) == 4);
  THES_CHECK(ms.axis_size(thes::index_tag<0>) == 2);
  THES_CHECK(ms.axis_size(thes::index_tag<2>) == 4);

  // `from_size(d)` is the number of elements spanned by axes `d` and beyond.
  THES_CHECK(test::range_eq(ms.from_sizes(), std::array<Size, 4>{24, 12, 4, 1}));
  THES_CHECK(ms.from_size(0) == 24);
  THES_CHECK(ms.from_size(1) == 12);
  THES_CHECK(ms.from_size(2) == 4);
  THES_CHECK(ms.from_size(3) == 1);
  THES_CHECK(ms.from_size(thes::index_tag<1>) == 12);

  // `after_size(d)` is the stride of axis `d`, i.e. `from_size(d + 1)`.
  THES_CHECK(ms.after_size(0) == 12);
  THES_CHECK(ms.after_size(1) == 4);
  THES_CHECK(ms.after_size(2) == 1);
  THES_CHECK(ms.after_size(thes::index_tag<0>) == 12);
  THES_CHECK(ms.after_size(thes::index_tag<2>) == 1);
}

/** Checks that equality compares the axis sizes. */
THES_TEST_CASE("equality compares the sizes", "[utility][multi-size]") {
  const Basic ms{sizes};
  THES_CHECK(ms == Basic{sizes});
  THES_CHECK((ms != Basic{Axes{2, 3, 5}}));
  THES_CHECK((ms != Basic{Axes{4, 3, 2}}));
}

/** Checks a degenerate one-element space, where every stride is one. */
THES_TEST_CASE("a single-element space", "[utility][multi-size]") {
  const Basic ms{Axes{1, 1, 1}};
  THES_CHECK(ms.total_size() == 1);
  THES_CHECK(test::range_eq(ms.from_sizes(), std::array<Size, 4>{1, 1, 1, 1}));
  THES_CHECK(ms.pos_to_index(Axes{0, 0, 0}) == 0);
}

//==================================================================================================
// MultiSize
//==================================================================================================

/** Checks that positions and indices convert into each other, in row-major order. */
THES_TEST_CASE("positions and indices round-trip", "[utility][multi-size]") {
  const Multi ms{sizes};
  THES_REQUIRE(ms.total_size() == 24);

  // The first axis varies slowest, the last one fastest.
  THES_CHECK(ms.pos_to_index(Axes{0, 0, 0}) == 0);
  THES_CHECK(ms.pos_to_index(Axes{0, 0, 1}) == 1);
  THES_CHECK(ms.pos_to_index(Axes{0, 1, 0}) == 4);
  THES_CHECK(ms.pos_to_index(Axes{1, 0, 0}) == 12);
  THES_CHECK(ms.pos_to_index(Axes{1, 2, 3}) == 23);

  for (Size index = 0; index < ms.total_size(); ++index) {
    const Axes pos = ms.index_to_pos(index);
    THES_CHECK(pos[0] < 2);
    THES_CHECK(pos[1] < 3);
    THES_CHECK(pos[2] < 4);
    THES_CHECK(ms.pos_to_index(pos) == index);
  }
}

/** Checks that the per-axis accessors agree with the full position. */
THES_TEST_CASE("index_to_axis_index agrees with index_to_pos", "[utility][multi-size]") {
  const Multi ms{sizes};

  for (Size index = 0; index < ms.total_size(); ++index) {
    const Axes pos = ms.index_to_pos(index);

    THES_CHECK(ms.index_to_axis_index(index, 0) == pos[0]);
    THES_CHECK(ms.index_to_axis_index(index, 1) == pos[1]);
    THES_CHECK(ms.index_to_axis_index<0>(index) == pos[0]);
    THES_CHECK(ms.index_to_axis_index<1>(index) == pos[1]);
    // The last axis has no stride to divide by, so it takes a separate code path.
    THES_CHECK(ms.index_to_axis_index<2>(index) == pos[2]);
  }
}

//==================================================================================================
// SubMultiSize
//==================================================================================================

/** Checks the offsets, extents and half-open axis ranges of a sub-box. */
THES_TEST_CASE("a sub-box reports its bounds", "[utility][multi-size]") {
  const Sub2 sub{std::array<Size, 2>{1, 2}, std::array<Size, 2>{2, 3}};

  THES_CHECK(test::range_eq(sub.begins(), std::array<Size, 2>{1, 2}));
  THES_CHECK(test::range_eq(sub.ends(), std::array<Size, 2>{3, 5}));
  THES_CHECK(test::range_eq(sub.sizes(), std::array<Size, 2>{2, 3}));
  THES_CHECK(sub.total_size() == 6);

  THES_CHECK(sub.axis_begin(0) == 1);
  THES_CHECK(sub.axis_end(0) == 3);
  THES_CHECK(sub.axis_size(0) == 2);
  THES_CHECK(sub.axis_begin(thes::index_tag<1>) == 2);
  THES_CHECK(sub.axis_end(thes::index_tag<1>) == 5);
  THES_CHECK(sub.axis_size(thes::index_tag<1>) == 3);

  THES_CHECK(test::range_eq(sub.axis_range(0), std::array<Size, 2>{1, 2}));
  THES_CHECK(test::range_eq(sub.axis_range(1), std::array<Size, 3>{2, 3, 4}));
  THES_CHECK(test::range_eq(sub.axis_ranges()[0], std::array<Size, 2>{1, 2}));
}

/** Checks that local and global coordinates convert into each other. */
THES_TEST_CASE("local and global positions convert", "[utility][multi-size]") {
  using Pos = std::array<Size, 2>;
  const Sub2 sub{Pos{1, 2}, Pos{2, 3}};

  for (Size index = 0; index < sub.total_size(); ++index) {
    const Pos local = sub.local_index_to_pos(index);
    const Pos global = sub.local_index_to_global_pos(index);

    THES_CHECK(test::range_eq(sub.local_to_global_pos(local), global));
    THES_CHECK(test::range_eq(sub.global_to_local_pos(global), local));
    THES_CHECK(sub.local_pos_to_index(local) == index);
    THES_CHECK(sub.global_pos_to_local_index(global) == index);
    THES_CHECK(sub.contains(global));
  }
}

/** Checks that `contains` follows the half-open axis ranges. */
THES_TEST_CASE("contains follows the half-open bounds", "[utility][multi-size]") {
  using Pos = std::array<Size, 2>;
  const Sub2 sub{Pos{1, 2}, Pos{2, 3}};

  THES_CHECK(sub.contains(Pos{1, 2}));
  THES_CHECK(sub.contains(Pos{2, 4}));
  THES_CHECK(!sub.contains(Pos{0, 2}));
  THES_CHECK(!sub.contains(Pos{3, 2}));
  THES_CHECK(!sub.contains(Pos{1, 1}));
  THES_CHECK(!sub.contains(Pos{1, 5}));
}

/** Checks that `reflect_into` mirrors out-of-bounds coordinates back into the box. */
THES_TEST_CASE("reflect_into mirrors at the boundaries", "[utility][multi-size]") {
  using Pos = std::array<Size, 2>;
  // Axis 0 spans [1, 3), axis 1 spans [2, 5).
  const Sub2 sub{Pos{1, 2}, Pos{2, 3}};

  // Coordinates already inside are left alone.
  THES_CHECK(test::range_eq(sub.reflect_into(Pos{1, 2}), Pos{1, 2}));
  THES_CHECK(test::range_eq(sub.reflect_into(Pos{2, 4}), Pos{2, 4}));

  // One step past a bound maps to the element just inside it.
  THES_CHECK(test::range_eq(sub.reflect_into(Pos{0, 1}), Pos{1, 2}));
  THES_CHECK(test::range_eq(sub.reflect_into(Pos{3, 5}), Pos{2, 4}));
  THES_CHECK(test::range_eq(sub.reflect_into(Pos{0, 6}), Pos{1, 3}));

  // Each axis is reflected only once, so a coordinate lands inside the box exactly if it is at
  // most one box width outside it, i.e. within `[begin - size, end + size)`.
  for (Size i = 0; i < 5; ++i) {
    for (Size j = 0; j < 8; ++j) {
      THES_CHECK(sub.contains(sub.reflect_into(Pos{i, j})));
    }
  }

  // Further out, a single reflection is not enough, and the result stays outside.
  THES_CHECK(!sub.contains(sub.reflect_into(Pos{5, 2})));
}

/** Checks that `reflect` mirrors only the selected axes, within the box. */
THES_TEST_CASE("reflect mirrors the selected axes", "[utility][multi-size]") {
  using Pos = std::array<Size, 2>;
  using Sel = std::array<bool, 2>;
  const Sub2 sub{Pos{1, 2}, Pos{2, 3}};

  THES_CHECK(test::range_eq(sub.reflect(Pos{1, 2}, Sel{false, false}), Pos{1, 2}));
  THES_CHECK(test::range_eq(sub.reflect(Pos{1, 2}, Sel{true, false}), Pos{2, 2}));
  THES_CHECK(test::range_eq(sub.reflect(Pos{1, 2}, Sel{false, true}), Pos{1, 4}));
  THES_CHECK(test::range_eq(sub.reflect(Pos{1, 2}, Sel{true, true}), Pos{2, 4}));

  // Reflecting twice along the same axes is the identity.
  for (Size i = 1; i < 3; ++i) {
    for (Size j = 2; j < 5; ++j) {
      const Pos pos{i, j};
      THES_CHECK(
        test::range_eq(sub.reflect(sub.reflect(pos, Sel{true, true}), Sel{true, true}), pos));
    }
  }
}

/** Checks that `expand` grows the box by the requested amount, clipped to the world. */
THES_TEST_CASE("expand is clipped to the surrounding world", "[utility][multi-size]") {
  const Multi world{Axes{10, 10, 10}};
  const Sub3 sub{Axes{1, 2, 3}, Axes{2, 2, 2}};

  const Sub3 grown = sub.expand(2, world);
  // Axis 0 would start at -1 and axis 1 at 0, so both are clamped to zero.
  THES_CHECK(test::range_eq(grown.begins(), Axes{0, 0, 1}));
  THES_CHECK(test::range_eq(grown.ends(), Axes{5, 6, 7}));

  // Expanding by zero leaves the box unchanged.
  const Sub3 same = sub.expand(0, world);
  THES_CHECK(test::range_eq(same.begins(), sub.begins()));
  THES_CHECK(test::range_eq(same.ends(), sub.ends()));

  // A large amount saturates at the world’s bounds on both sides.
  const Sub3 full = sub.expand(100, world);
  THES_CHECK(test::range_eq(full.begins(), Axes{0, 0, 0}));
  THES_CHECK(test::range_eq(full.ends(), Axes{10, 10, 10}));
  THES_CHECK(full.total_size() == world.total_size());
}

/** Checks that a sub-box covering the whole world indexes exactly like the world itself. */
THES_TEST_CASE("a full-extent sub-box matches its world", "[utility][multi-size]") {
  const Multi world{sizes};
  const Sub3 sub{Axes{0, 0, 0}, sizes};

  THES_REQUIRE(sub.total_size() == world.total_size());
  for (Size index = 0; index < world.total_size(); ++index) {
    THES_CHECK(test::range_eq(sub.local_index_to_global_pos(index), world.index_to_pos(index)));
    THES_CHECK(sub.global_pos_to_local_index(world.index_to_pos(index)) == index);
  }
}
} // namespace

THES_TEST_MAIN()
