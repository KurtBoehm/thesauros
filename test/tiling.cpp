#include <array>
#include <cstddef>
#include <optional>
#include <utility>

#include "thesauros/algorithms.hpp"
#include "thesauros/algorithms/ranges/for-each-tile.hpp"
#include "thesauros/utility.hpp"
#include "thesauros/utility/static-map.hpp"

int main() {
  using namespace thes::literals;
  namespace star = thes::star;

  static constexpr std::array sizes{16_uz, 8_uz, 12_uz};
  static constexpr thes::MultiSize ms{sizes};
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

  static constexpr auto make_index_pos = [](std::size_t ref_idx, auto dir) {
    std::size_t index = 0;
    std::optional<thes::IndexPosition<std::size_t, 3>> out = std::nullopt;
    thes::iterate_tile<dir>(ms,
                            std::array{std::make_pair(4_uz, 8_uz), std::make_pair(4_uz, 8_uz),
                                       std::make_pair(0_uz, 4_uz)},
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
    static_assert(ms.index_to_pos(index_pos.index) == index_pos.position);
    static_assert(index_pos.position == std::array{7_uz, 7_uz, 3_uz});
  }
  {
    static constexpr auto index_pos =
      make_index_pos(16, thes::auto_tag<thes::IterDirection::BACKWARD>);
    static_assert(ms.index_to_pos(index_pos.index) == index_pos.position);
    static_assert(index_pos.position == std::array{6_uz, 7_uz, 3_uz});
  }
  {
    static constexpr auto index_pos =
      make_index_pos(16, thes::auto_tag<thes::IterDirection::FORWARD>);
    static_assert(ms.index_to_pos(index_pos.index) == index_pos.position);
    static_assert(index_pos.position == std::array{5_uz, 4_uz, 0_uz});
  }
}
