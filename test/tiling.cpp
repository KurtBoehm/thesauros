#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <vector>

#include "thesauros/algorithms.hpp"
#include "thesauros/format.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/static-ranges.hpp"
#include "thesauros/test.hpp"
#include "thesauros/types.hpp"
#include "thesauros/utility.hpp"

#define COMPILE_TIME true

namespace {
struct Idx {
  constexpr explicit Idx(size_t pidx) : idx(pidx) {}
  size_t idx;
};

inline constexpr auto forward = thes::auto_tag<thes::IterDirection::FORWARD>;
inline constexpr auto backward = thes::auto_tag<thes::IterDirection::BACKWARD>;

inline constexpr auto def_rng = thes::star::transform([](auto s) { return thes::range(s); });

constexpr auto tiled_base(auto tag, auto sizes, auto ranges, auto tile_sizes, auto map) {
  std::vector<std::size_t> idxs{};
  thes::tiled_for_each<tag>(
    thes::MultiSize{sizes}, ranges, tile_sizes, map,
    [&](auto pos) { idxs.insert(idxs.end(), {pos.index.idx, pos.index.idx + 1}); },
    [&](auto pos, auto part) {
      assert(part == 1);
      idxs.push_back(pos.index.idx);
    },
    thes::index_tag<2>, thes::type_tag<Idx>);

  const auto min = *std::ranges::min_element(idxs);
  const auto max = *std::ranges::max_element(idxs);
  if (!std::is_constant_evaluated()) {
    fmt::print("{}: [{}, {}] → {}, {}\n\n", idxs, min, max, max - min, idxs.size());
  }
  THES_ASSERT(max + 1 - min == idxs.size());
}

void test_scalar() {
  using namespace thes::literals;
  static constexpr std::array sizes{16_uz, 8_uz, 12_uz};
  static constexpr thes::MultiSize multi_size{sizes};
  static constexpr auto tile_sizes = thes::star::constant<3>(4_uz);

  static_assert([] {
    std::size_t num = 0;
    thes::for_each_tile<thes::IterDirection::FORWARD>(
      sizes | def_rng, tile_sizes, thes::StaticMap{}, [&num](auto, auto, auto) { ++num; });
    return num;
  }() == 24);
  static constexpr auto tiles = [] {
    std::size_t num = 0;
    std::array<thes::IotaRange<std::size_t>, 24_uz * 3_uz> arr{};
    thes::for_each_tile<thes::IterDirection::FORWARD>(
      sizes | def_rng, tile_sizes, thes::StaticMap{}, [&arr, &num](auto t1, auto t2, auto t3) {
        arr[3 * num + 0] = t1;
        arr[3 * num + 1] = t2;
        arr[3 * num + 2] = t3;
        ++num;
      });
    return arr;
  }();
  static_assert(tiles[27] == thes::range(4_uz, 8_uz));
  static_assert(tiles[28] == thes::range(4_uz, 8_uz));
  static_assert(tiles[29] == thes::range(0_uz, 4_uz));

  // Returns the index-position of the ref_idx-th position in the iteration order specified by dir
  static constexpr auto make_index_pos = [](std::size_t ref_idx, auto dir) {
    std::size_t index = 0;
    std::optional<thes::IndexPosition<std::size_t, std::array<std::size_t, 3>>> out = std::nullopt;
    thes::tile_for_each<dir>(multi_size, std::array{tiles[27], tiles[28], tiles[29]},
                             [ref_idx, &index, &out](auto index_pos) {
                               if (index == ref_idx) {
                                 out = index_pos;
                               }
                               ++index;
                             });
    return out.value();
  };
  {
    static constexpr auto index_pos =
      make_index_pos(0, thes::auto_tag<thes::IterDirection::BACKWARD>);
    static_assert(multi_size.index_to_pos(index_pos.index) == index_pos.position);
    static_assert(index_pos.position == std::array{7_uz, 7_uz, 3_uz});
  }
  {
    static constexpr auto index_pos =
      make_index_pos(16, thes::auto_tag<thes::IterDirection::BACKWARD>);
    static_assert(multi_size.index_to_pos(index_pos.index) == index_pos.position);
    static_assert(index_pos.position == std::array{6_uz, 7_uz, 3_uz});
  }
  {
    static constexpr auto index_pos =
      make_index_pos(16, thes::auto_tag<thes::IterDirection::FORWARD>);
    static_assert(multi_size.index_to_pos(index_pos.index) == index_pos.position);
    static_assert(index_pos.position == std::array{5_uz, 4_uz, 0_uz});
  }

  auto lambda = [](auto tag, auto ranges, auto map) {
    std::vector<std::size_t> idxs{};
    thes::tiled_for_each<tag>(
      thes::MultiSize{sizes}, ranges, tile_sizes, map,
      [&](auto pos) { idxs.push_back(pos.index.idx); }, thes::type_tag<Idx>);

    const auto min = *std::ranges::min_element(idxs);
    const auto max = *std::ranges::max_element(idxs);
    return max + 1 - min == idxs.size();
  };
  static_assert(lambda(forward, sizes | def_rng, thes::StaticMap{}));
  static_assert(lambda(backward, sizes | def_rng, thes::StaticMap{}));
}

void test_vectorized() {
  using namespace thes::literals;
  static constexpr auto sizes = std::array{15_uz, 8_uz, 13_uz};
  static constexpr auto tile_sizes = thes::star::constant<3>(4_uz);

#if COMPILE_TIME
  auto tiled_consteval = [&](auto tag, auto ranges, auto map) consteval {
    tiled_base(tag, sizes, ranges, tile_sizes, map);
  };
#endif
  auto tiled = [&](auto tag, auto ranges, auto map) {
    tiled_base(tag, sizes, ranges.value, tile_sizes, map.value);
#if COMPILE_TIME
    tiled_consteval(tag, ranges.value, map.value);
#endif
  };

  {
    static constexpr auto ranges = thes::auto_tag<(sizes | def_rng)>;

    tiled(forward, ranges, thes::static_map_tag<>);
    tiled(backward, ranges, thes::static_map_tag<>);
    tiled(forward, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
    tiled(backward, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
  }
  {
    static constexpr auto ranges = thes::auto_tag<thes::star::index_transform<3>(
      [](auto idx) { return thes::range(std::get<idx>(sizes)); })>;

    tiled(forward, ranges, thes::static_map_tag<>);
    tiled(backward, ranges, thes::static_map_tag<>);
    tiled(forward, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
    tiled(backward, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
  }
}

constexpr void test_small() {
  using namespace thes::literals;
  // tiled_base(
  //   forward, std::array{4_u32, 9_u32, 7_u32},
  //   std::array{thes::range(2_u32, 4_u32), thes::range(0_u32, 9_u32), thes::range(0_u32, 7_u32)},
  //   std::array{8_u32, 8_u32, 8_u32}, thes::StaticMap{});

  std::vector<thes::u32> idxs{};
  thes::tiled_for_each<backward>(
    thes::MultiSize{std::array{4_u32, 9_u32, 7_u32}},
    std::array{thes::range(2_u32, 4_u32), thes::range(0_u32, 9_u32), thes::range(0_u32, 7_u32)},
    std::array{8_u32, 8_u32, 8_u32}, thes::StaticMap{},
    [&](auto pos) { idxs.push_back(pos.index); });

  const auto min = *std::ranges::min_element(idxs);
  const auto max = *std::ranges::max_element(idxs);
  if (!std::is_constant_evaluated()) {
    fmt::print("{}: [{}, {}] → {}, {}\n\n", idxs, min, max, max - min, idxs.size());
  }
  THES_ASSERT(max + 1 - min == idxs.size());
}
} // namespace

int main() {
  test_scalar();
  test_vectorized();

#if COMPILE_TIME
  []() consteval { test_small(); }();
#endif
  test_small();
}
