#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <set>
#include <utility>

#include "thesauros/algorithms.hpp"
#include "thesauros/io.hpp"
#include "thesauros/test.hpp"
#include "thesauros/utility.hpp"

static constexpr auto forward = thes::auto_tag<thes::IterDirection::FORWARD>;
static constexpr auto backward = thes::auto_tag<thes::IterDirection::BACKWARD>;

static constexpr auto def_pairs =
  thes::star::transform([](auto s) { return std::make_pair(std::size_t{0}, s); });

int main() {
  using namespace thes::literals;
  namespace star = thes::star;

  static constexpr std::array sizes{16_uz, 8_uz, 12_uz};
  static constexpr thes::MultiSize multi_size{sizes};
  static constexpr auto tile_sizes = star::constant<3>(4_uz);
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
    std::optional<thes::IndexPosition<std::size_t, 3>> out = std::nullopt;
    thes::iterate_tile<dir>(multi_size, std::array{tiles[27], tiles[28], tiles[29]},
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

  auto lambda = [&](auto tag, auto ms, auto ranges, auto map) {
    static constexpr auto tag2 = thes::index_tag<2>;

    std::set<std::size_t> idxs{};
    thes::for_each_tile_vectorized<tag>(
      ranges, tile_sizes, map,
      [&](auto... args) {
        thes::iterate_tile<tag>(
          ms, std::array{args...},
          [&](auto pos) {
            idxs.insert({pos.index, pos.index + 1});
          },
          thes::NoOp{}, tag2, thes::false_tag);
      },
      [&](auto... args) {
        thes::iterate_tile<tag>(
          ms, std::array{args...},
          [&](auto pos) {
            idxs.insert({pos.index, pos.index + 1});
          },
          [&](auto pos, auto part) {
            assert(part == 1);
            idxs.insert({pos.index});
          },
          tag2, thes::true_tag);
      },
      tag2);

    const auto min = *std::ranges::min_element(idxs);
    const auto max = *std::ranges::max_element(idxs);
    std::cout << thes::print(idxs) << ": [" << min << ", " << max << "] → " << (max - min) << ", "
              << idxs.size() << "\n\n";
    THES_ASSERT(max + 1 - min == idxs.size());
  };

  {
    static constexpr thes::MultiSize ms{std::array{15_uz, 8_uz, 13_uz}};
    static constexpr auto ranges = ms.sizes() | def_pairs;
    lambda(forward, ms, ranges, thes::StaticMap{});
    lambda(backward, ms, ranges, thes::StaticMap{});
    lambda(forward, ms, ranges, thes::StaticMap{thes::static_key<0_uz> = 1_uz});
    lambda(backward, ms, ranges, thes::StaticMap{thes::static_key<0_uz> = 1_uz});
  }
  {
    static constexpr thes::MultiSize ms{std::array{15_uz, 8_uz, 13_uz}};
    static constexpr auto ranges = thes::star::index_transform<3>([](auto idx) {
      return std::make_pair(std::size_t{idx == 0} * 4, ms.axis_size(thes::index_tag<idx>));
    });
    lambda(forward, ms, ranges, thes::StaticMap{});
    lambda(backward, ms, ranges, thes::StaticMap{});
    lambda(forward, ms, ranges, thes::StaticMap{thes::static_key<0_uz> = 1_uz});
    lambda(backward, ms, ranges, thes::StaticMap{thes::static_key<0_uz> = 1_uz});
  }
}
