#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "thesauros/algorithms.hpp"
#include "thesauros/algorithms/ranges/for-each-tile.hpp"
#include "thesauros/io.hpp"
#include "thesauros/test.hpp"
#include "thesauros/utility.hpp"

struct Idx {
  constexpr explicit Idx(size_t idx) : idx(idx) {}
  size_t idx;
};

static constexpr auto forward = thes::auto_tag<thes::IterDirection::FORWARD>;
static constexpr auto backward = thes::auto_tag<thes::IterDirection::BACKWARD>;

static constexpr auto def_pairs =
  thes::star::transform([](auto s) { return std::make_pair(std::size_t{0}, s); });

static constexpr auto tiled_base(auto tag, auto sizes, auto ranges, auto tile_sizes, auto map) {
  std::vector<std::size_t> idxs{};
  thes::tiled_for_each<tag>(
    thes::MultiSize{sizes}, ranges, tile_sizes, map,
    [&](auto pos) {
      idxs.insert(idxs.end(), {pos.index, pos.index + 1});
    },
    [&](auto pos, auto part) {
      assert(part == 1);
      idxs.push_back(pos.index);
    },
    thes::index_tag<2>);

  const auto min = *std::ranges::min_element(idxs);
  const auto max = *std::ranges::max_element(idxs);
  if (!std::is_constant_evaluated()) {
    std::cout << thes::print(idxs) << ": [" << min << ", " << max << "] → " << (max - min) << ", "
              << idxs.size() << "\n\n";
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
      sizes | def_pairs, tile_sizes, thes::StaticMap{}, [&num](auto, auto, auto) { ++num; });
    return num;
  }() == 24);
  static constexpr auto tiles = [] {
    std::size_t num = 0;
    std::array<std::pair<std::size_t, std::size_t>, 24_uz * 3_uz> arr{};
    thes::for_each_tile<thes::IterDirection::FORWARD>(
      sizes | def_pairs, tile_sizes, thes::StaticMap{}, [&arr, &num](auto t1, auto t2, auto t3) {
        arr[3 * num + 0] = t1;
        arr[3 * num + 1] = t2;
        arr[3 * num + 2] = t3;
        ++num;
      });
    return arr;
  }();
  static_assert(tiles[27] == std::make_pair(4_uz, 8_uz));
  static_assert(tiles[28] == std::make_pair(4_uz, 8_uz));
  static_assert(tiles[29] == std::make_pair(0_uz, 4_uz));

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
  static_assert(lambda(forward, sizes | def_pairs, thes::StaticMap{}));
  static_assert(lambda(backward, sizes | def_pairs, thes::StaticMap{}));
}

void test_vectorized() {
  using namespace thes::literals;
  static constexpr auto sizes = thes::auto_tag<std::array{15_uz, 8_uz, 13_uz}>;
  static constexpr auto tile_sizes = thes::star::constant<3>(4_uz);

  auto tiled_consteval = [&](auto tag, auto sizes, auto ranges, auto map) consteval {
    tiled_base(tag, sizes, ranges, tile_sizes, map);
  };
  auto tiled = [&](auto tag, auto sizes, auto ranges, auto map) {
    tiled_base(tag, sizes.value, ranges.value, tile_sizes, map.value);
    tiled_consteval(tag, sizes.value, ranges.value, map.value);
  };

  {
    static constexpr auto ranges = thes::auto_tag<decltype(sizes)::value | def_pairs>;

    tiled(forward, sizes, ranges, thes::static_map_tag<>);
    tiled(backward, sizes, ranges, thes::static_map_tag<>);
    tiled(forward, sizes, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
    tiled(backward, sizes, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
  }
  {
    static constexpr auto ranges = thes::auto_tag<thes::star::index_transform<3>([](auto idx) {
      return std::make_pair(std::size_t{idx == 0} * 4, std::get<idx>(decltype(sizes)::value));
    })>;

    tiled(forward, sizes, ranges, thes::static_map_tag<>);
    tiled(backward, sizes, ranges, thes::static_map_tag<>);
    tiled(forward, sizes, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
    tiled(backward, sizes, ranges, thes::static_map_tag<thes::static_kv<0_uz, 1_uz>>);
  }
}

int main() {
  test_scalar();
  test_vectorized();
}
