// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "thesauros/algorithms.hpp"
#include "thesauros/format.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/static-ranges.hpp"
#include "thesauros/types.hpp"
#include "thesauros/utility.hpp"

int main() {
  using namespace thes::literals;
  namespace star = thes::star;

  // std::array, std::tuple, !std::vector
  {
    using Range = std::array<int, 4>;
    static constexpr Range arr{0, 4, 3, 1};

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(arr) == 1);
    static_assert(std::same_as<star::Value<Range>, int>);
  }
  {
    using Range = std::tuple<>;
    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 0);
    static_assert(!star::HasValue<Range>);
  }
  {
    using Range = std::tuple<int>;
    static constexpr Range tup{1};

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 1);
    static_assert(star::get_at<0>(tup) == 1);
    static_assert(star::HasValue<Range>);
  }
  {
    using Range = std::tuple<int, float, double>;
    static constexpr Range tup{1, 2.0F, 3.0};

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(tup) == 2.0F);
    static_assert(!star::HasValue<Range>);
  }
  {
    using Range = std::vector<int>;
    static_assert(!star::AnyStaticRange<Range>);
  }

  // Tuple
  {
    using Range = thes::Tuple<int, float, double>;
    static constexpr Range tup{1, 2.0F, 3.0};
    [[maybe_unused]] static constexpr thes::AutoTag<tup> sv{};

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(tup) == 2.0F);
    static_assert(!star::HasValue<Range>);
  }

  // constant
  {
    static constexpr auto con = star::constant<5>(1);
    using Range = decltype(con);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 5);
    static_assert(star::get_at<4>(con) == 1);
  }

  // generate
  {
    static constexpr auto con = star::generate<5>([] { return 1; });
    using Range = decltype(con);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 5);
    static_assert(star::get_at<4>(con) == 1);
  }
  {
    static constexpr auto con = star::generate<int, 0>([] { return 1; });
    using Range = decltype(con);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 0);
  }

  // iota
  {
    static constexpr auto con = star::iota<0, 5>;
    using Range = decltype(con);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 5);
    static_assert(star::get_at<4>(con) == 4);
  }

  // enumerate
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto enu = arr | star::enumerate<std::size_t>;
    static constexpr auto ref = 1;
    using Range = decltype(enu);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(enu) == std::make_pair(thes::index_tag<3>, std::cref(ref)));
    static_assert(!star::HasValue<Range>);
  }
  {
    static constexpr std::tuple<int, float, double> tup{1, 2.0F, 3.0};
    static constexpr auto enu = tup | star::enumerate<int>;
    static constexpr auto ref = 2.0F;
    using Range = decltype(enu);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(enu) == std::make_pair(thes::auto_tag<1>, std::cref(ref)));
    static_assert(!star::HasValue<Range>);
  }
  {
    static constexpr auto con = star::constant<5>(1);
    static constexpr auto enu = con | star::enumerate<int>;
    using Range = decltype(enu);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 5);
    static_assert(star::get_at<4>(enu) == std::make_pair(thes::auto_tag<4>, 1));
  }

  // transform
  {
    static constexpr auto map = star::iota<0, 4> | star::transform([](auto v) { return 2 * v; });
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, std::size_t>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(map) == 6);
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map =
      arr | star::transform([](auto v) { return 2 * v; }) | star::enumerate<int>;
    using Range = decltype(map);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(map) == std::make_pair(thes::auto_tag<3>, 2));
    static_assert(!star::HasValue<Range>);
  }
  {
    static constexpr thes::Tuple<int, float, double> tup{1, 2.0F, 3.0};
    static constexpr auto map = tup | star::transform([](auto v) { return 2 * v; });
    using Range = decltype(map);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(map) == 4.0F);
    static_assert(!star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr std::tuple tup{1, 2.0F, 3.0, 4U};
    static constexpr auto lambda = [](auto v1, auto v2) { return v1 * int(v2); };
    // TODO gcc does not like “lambda” here, as it claims the reference is not a constant expression
    static constexpr auto map1 =
      star::transform([](auto v1, auto v2) { return v1 * int(v2); })(arr, tup);
    static constexpr auto map2 = star::transform(lambda, arr, tup);
    static_assert((map1 | star::to_array) == (map2 | star::to_array));
    static constexpr auto map = map2;
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<1>(map) == 8.0F);
    static_assert(star::HasValue<Range>);
  }
  {
    using namespace thes::literals;

    static constexpr auto map = star::index_transform<4>([](auto v) { return v * v; });
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, std::size_t>);
    static_assert(star::size<Range> == 4);
    static_assert((map | star::to_array) == std::array{0_uz, 1_uz, 4_uz, 9_uz});
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map =
      arr | (star::transform([](auto v) { return 2 * v; }) | star::enumerate<int>);
    using Range = decltype(map);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(map) == std::make_pair(thes::auto_tag<3>, 2));
    static_assert(!star::HasValue<Range>);
  }
  {
    static constexpr auto trans = std::tuple{} | star::transform<int>([](int a) { return a + 1; });
    using Range = decltype(trans);
    static_assert(star::HasValue<Range>);
    static_assert(std::same_as<star::Value<Range>, int>);
  }
  {
    static int a = 1;
    static constexpr auto trans =
      std::tuple<int&>{a} | star::transform([](int& i) { return i + 1; });
    using Range = decltype(trans);
    static_assert(star::HasValue<Range>);
    static_assert(std::same_as<star::RawValue<Range>, int>);
  }

  // filter
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map = arr | star::only_idxs<0, 2>;
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 2);
    static_assert(star::get_at<1>(map) == 3);
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map =
      arr | star::only_idxs<0, 2> | star::transform([](auto v) { return 2 * v; });
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 2);
    static_assert(star::get_at<1>(map) == 6);
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map = arr | star::only_range<std::array{2}>;
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 1);
    static_assert(star::get_at<0>(map) == 3);
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map = arr | star::all_except_idxs<2>;
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 3);
    static_assert((map | star::to_array) == std::array{0, 4, 1});
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map = arr | star::all_except_range<std::array{1}>;
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 3);
    static_assert((map | star::to_array) == std::array{0, 3, 1});
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map = arr | star::sub_range<1, 3>;
    using Range = decltype(map);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 2);
    static_assert((map | star::to_array) == std::array{4, 3});
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr auto rng =
      star::iota<3, 10> |
      star::filter<[](auto /*idx*/, auto type) { return type.construct() % 2 == 0; }>;
    using Range = decltype(rng);
    static_assert(!star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert((rng | star::transform([](auto i) { return i.value; }) | star::to_array) ==
                  std::array{4_uz, 6_uz, 8_uz});
  }

  // join
  {
    static constexpr auto j =
      star::joined(std::array{0, 4, 3, 1}, std::array{4, 0}, std::array{3, 1});
    using Range = decltype(j);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 8);
    static_assert(star::get_at<1>(j) == 4);
    static_assert((j | star::to_array) == std::array{0, 4, 3, 1, 4, 0, 3, 1});
    static_assert(star::HasValue<Range>);
  }
  {
    static constexpr auto j =
      std::tuple{std::array{0, 4, 3, 1}, std::array{4, 0}, std::array{3, 1}} | star::join;
    using Range = decltype(j);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 8);
    static_assert(star::get_at<1>(j) == 4);
    static_assert((j | star::to_array) == std::array{0, 4, 3, 1, 4, 0, 3, 1});
    static_assert(star::HasValue<Range>);
  }

  // flatten
  {
    static constexpr auto j =
      star::flattened(std::tuple{std::array{0, 4, 3, 1}, std::array{4, 0}, std::array{3, 1}});
    using Range = decltype(j);

    static_assert(star::TypedStaticRange<Range, int>);
    static_assert(star::size<Range> == 8);
    static_assert(star::get_at<1>(j) == 4);
    static_assert((j | star::to_array) == std::array{0, 4, 3, 1, 4, 0, 3, 1});
    static_assert(star::HasValue<Range>);
  }

  // zip
  {
    static constexpr auto r1 = std::array{1, 2, 3};
    static constexpr auto r2 = std::tuple{1U, 2, 3.F};
    static constexpr auto z = thes::star::zip(r1, r2);
    using Range = decltype(z);

    static_assert(star::AnyStaticRange<Range> && !star::AnyTypedStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(z) == thes::Tuple{std::get<1>(r1), std::get<1>(r2)});
    static_assert((z | star::to_tuple) ==
                  thes::Tuple{thes::Tuple{std::get<0>(r1), std::get<0>(r2)},
                              thes::Tuple{std::get<1>(r1), std::get<1>(r2)},
                              thes::Tuple{std::get<2>(r1), std::get<2>(r2)}});
    static_assert(!star::HasValue<Range>);
  }

  // left_reduce
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto red =
      arr | star::transform([](auto v) { return 2 * v; }) | star::left_reduce(std::plus<>{}, 0);
    static_assert(red == 16);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto red =
      arr | (star::transform([](auto v) { return 2 * v; }) | star::left_reduce(std::plus<>{}));
    static_assert(red == 16);
  }
  {
    static constexpr std::array<int, 0> arr{};
    static constexpr auto red =
      arr | star::transform([](auto v) { return 2 * v; }) | star::left_reduce(std::plus<>{}, 0);
    static_assert(red == 0);
  }
  {
    static constexpr std::array arr{4, 3, 1};
    static_assert((arr | star::minimum) == 1);
    static_assert((arr | star::maximum) == 4);
  }

  // right_reduce
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto red =
      arr | star::transform([](auto v) { return 2 * v; }) | star::right_reduce(std::plus<>{}, 0);
    static_assert(red == 16);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto red =
      arr | (star::transform([](auto v) { return 2 * v; }) | star::right_reduce(std::plus<>{}));
    static_assert(red == 16);
  }

  // contains
  {
    static constexpr std::tuple<int, float, double> tup{1, 2.0F, 3.0};
    static_assert(tup | star::contains(1));
    static_assert(!(tup | star::contains(1.0F)));
  }
  {
    static_assert(star::get_at<0>(thes::star::iota<0, 1>) == 0);
    static_assert(star::size<decltype(thes::star::iota<0, 1>)> == 1);
    static_assert(thes::star::iota<0, 1> | star::contains(thes::index_tag<0>));
  }

  // to_array
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto red = star::to_array(arr | star::transform([](auto v) { return 2 * v; }));
    static_assert(red == std::array{0, 8, 6, 2});
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto red =
      arr | star::transform([](auto v) { return 2 * v; }) | star::to_array;
    static_assert(red == std::array{0, 8, 6, 2});
  }

  // to_tuple
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto red =
      arr | star::transform([](auto v) { return 2 * v; }) | star::to_tuple;
    static_assert(red == thes::Tuple{0, 8, 6, 2});
  }

  // for_each
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static_assert([&] {
      int sum = 0;
      arr | star::transform([](auto v) { return 2 * v; }) |
        star::for_each([&sum](auto v) { sum += v; });
      return sum;
    }() == 16);
  }

  // format
  {
    static constexpr std::array arr{0, 4, 3, 1};
    fmt::print("{}\n", arr);
    fmt::print("{}\n", arr | star::transform([](auto v) { return 2 * v; }));
  }

  // first_value
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static_assert((arr | star::transform([](int v) -> std::optional<int> {
                     if (v == 3) {
                       return 2 * v;
                     }
                     return std::nullopt;
                   }) |
                   star::first_value)
                    .value() == 6);
    static_assert([&] {
      int idx = 0;
      auto rng = std::array{0, 1, 0, 1} | star::transform([&](int v) -> std::optional<int> {
                   ++idx;
                   if (v == 0) {
                     return v;
                   }
                   return std::nullopt;
                 }) |
                 star::first_value;
      return std::make_pair(rng, idx);
    }() == std::make_pair(std::make_optional(0), 1));
    static_assert([&] {
      int idx = 0;
      auto rng = std::array{0, 1, 0, 1} | star::transform([&](int v) -> std::optional<int> {
                   ++idx;
                   if (v == 1) {
                     return v;
                   }
                   return std::nullopt;
                 }) |
                 star::first_value;
      return std::make_pair(rng, idx);
    }() == std::make_pair(std::make_optional(1), 2));
    static_assert([&] {
      int idx = 0;
      std::array{0, 1, 0, 1} | star::transform([&](int v) { idx += v; }) | star::first_value;
      return idx;
    }() == 2);
  }

  // apply
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static_assert((arr | star::transform([](auto v) { return 2 * v; }) |
                   star::apply([](auto... v) { return (... + v); })) == 16);
  }

  // multidim_for_each
  {
    static constexpr std::array ranges{thes::range(1, 3), thes::range(1, 2), thes::range(1, 4)};
    static_assert([] {
      int value = 0;
      thes::multidim_for_each(ranges,
                              [&value](auto v1, auto v2, auto v3) { value += v1 * v2 * v3; });
      return value;
    }() == 18);
  }
  {
    static constexpr std::array ranges{thes::range(1, 3), thes::range(1, 2), thes::range(1, 4)};
    static_assert([] {
      int value = 0;
      thes::multidim_for_each(ranges, thes::StaticMap{thes::static_key<2> = 2},
                              [&value](auto v1, auto v2, auto v3) { value += v1 * v2 * v3; });
      return value;
    }() == 6);
  }
  {
    using namespace thes::literals;

    static constexpr std::array sizes{3_uz, 2_uz, 4_uz};
    static_assert([] {
      auto value = 0_uz;
      thes::multidim_for_each_size(sizes,
                                   [&value](auto v1, auto v2, auto v3) { value += v1 * v2 * v3; });
      return value;
    }() == 18);
  }
  {
    using namespace thes::literals;

    static constexpr std::array sizes{3_uz, 2_uz, 4_uz};
    static_assert([] {
      auto value = 0_uz;
      thes::multidim_for_each_size(sizes, thes::StaticMap{thes::static_key<2> = 2_uz},
                                   [&value](auto v1, auto v2, auto v3) { value += v1 * v2 * v3; });
      return value;
    }() == 6);
  }
  {
    static_assert(std::array{1, 2, 3} | thes::star::all_different);
    static_assert(!(std::array{1, 2, 2} | thes::star::all_different));
    static_assert(!(std::array{1, 1, 2} | thes::star::all_different));
  }
  {
    static_assert(std::array{1, 1, 1} | thes::star::has_unique_value);
    static_assert(!(std::array{1, 2, 2} | thes::star::has_unique_value));
    static_assert(!(std::array{1, 1, 2} | thes::star::has_unique_value));
  }

  // index_to_position
  {
    using namespace thes::literals;

    static constexpr std::array sizes{3_uz, 2_uz, 4_uz};
    static_assert(star::index_to_position(1_uz, sizes) == std::array{0_uz, 0_uz, 1_uz});
    static_assert(star::index_to_position(6_uz, sizes) == std::array{0_uz, 1_uz, 2_uz});
  }

  // position_to_index and postfix_product_inclusive
  {
    using namespace thes::literals;

    static constexpr std::array sizes{3_uz, 2_uz, 4_uz};
    static constexpr auto prods = star::postfix_product_inclusive(sizes) | star::to_array;
    static_assert(prods == std::array{24_uz, 8_uz, 4_uz, 1_uz});

    static_assert(star::position_to_index(std::array{1_uz, 0_uz, 1_uz}, prods) == 9);
    static_assert(star::position_to_index(std::array{2_uz, 1_uz, 3_uz}, prods) == 23);
  }

  // BasicMultiSize
  {
    using namespace thes::literals;

    static constexpr thes::BasicMultiSize<std::size_t, 3> ms{{3_uz, 2_uz, 4_uz}};
    static_assert(ms.pos_to_index(std::array{1_uz, 0_uz, 1_uz}) == 9);
    static_assert(ms.pos_to_index(std::array{2_uz, 1_uz, 3_uz}) == 23);
  }

  // MultiSize
  {
    using namespace thes::literals;
    static constexpr thes::MultiSize<std::size_t, 3> ms{{3_uz, 2_uz, 4_uz}};

    static_assert(ms.index_to_pos(1_uz) == std::array{0_uz, 0_uz, 1_uz});
    static_assert(ms.index_to_axis_index<1>(1_uz) == 0);
    static_assert(ms.index_to_pos(6_uz) == std::array{0_uz, 1_uz, 2_uz});
    static_assert(ms.index_to_axis_index<1>(6_uz) == 1);
    static_assert(ms.pos_to_index(std::array{1_uz, 0_uz, 1_uz}) == 9);
    static_assert(ms.pos_to_index(std::array{2_uz, 1_uz, 3_uz}) == 23);
  }
}
