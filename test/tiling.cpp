#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <set>
#include <utility>

#include "thesauros/algorithms.hpp"
#include "thesauros/test.hpp"
#include "thesauros/utility.hpp"

int main() {
  using namespace thes::literals;
  namespace star = thes::star;

  static constexpr std::array sizes{16_uz, 8_uz, 12_uz};
  static constexpr thes::MultiSize multi_size{sizes};
  static constexpr auto tile_sizes = star::constant<3>(4_uz);
  static_assert([] {
    std::size_t num = 0;
    thes::for_each_tile<thes::IterDirection::FORWARD>(sizes, tile_sizes, thes::StaticMap{},
                                                      [&num](auto, auto, auto) { ++num; });
    return num;
  }() == 24);
  static constexpr auto tiles = [] {
    std::size_t num = 0;
    std::array<std::pair<std::size_t, std::size_t>, 24_uz * 3_uz> arr{};
    thes::for_each_tile<thes::IterDirection::FORWARD>(sizes, tile_sizes, thes::StaticMap{},
                                                      [&arr, &num](auto t1, auto t2, auto t3) {
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

  auto lambda = [&](auto tag) {
    static constexpr thes::MultiSize ms{std::array{15_uz, 8_uz, 13_uz}};
    static constexpr auto tag2 = thes::index_tag<2>;

    std::set<std::size_t> idxs{};
    thes::for_each_tile_vectorized<tag>(
      ms.sizes(), tile_sizes,
      [&](auto... args) {
        // std::cout << "full: " << thes::print(thes::Tuple{args...}) << '\n';
        thes::iterate_tile_full<tag>(
          ms, std::array{args...},
          [&](auto pos) {
            // std::cout << "ff" << pos.index << '\n';
            idxs.insert({pos.index, pos.index + 1});
          },
          tag2);
      },
      [&](auto... args) {
        // std::cout << "part: " << thes::print(thes::Tuple{args...}) << '\n';
        thes::iterate_tile_part<tag>(
          ms, std::array{args...},
          [&](auto pos) {
            // std::cout << "pf" << pos.index << '\n';
            idxs.insert({pos.index, pos.index + 1});
          },
          [&](auto pos, auto part) {
            // std::cout << "pp" << pos.index << '\n';
            assert(part == 1);
            idxs.insert({pos.index});
          },
          tag2);
      },
      tag2);

    THES_ASSERT(*std::ranges::max_element(idxs) + 1 == idxs.size());
  };

  lambda(thes::auto_tag<thes::IterDirection::FORWARD>);
  // std::cout << '\n';
  lambda(thes::auto_tag<thes::IterDirection::BACKWARD>);
}
