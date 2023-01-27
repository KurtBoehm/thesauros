#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <vector>

#include "thesauros/utility/static-ranges.hpp"
#include "thesauros/utility/static-ranges/ranges/map-values.hpp"

int main() {
  namespace star = thes::star;

  // std::array, std::tuple, !std::vector
  {
    using Range = std::array<int, 4>;
    static constexpr Range arr{0, 4, 3, 1};

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(arr) == 1);
    static_assert(std::same_as<star::ElementType<Range>, int>);
  }
  {
    using Range = std::tuple<>;
    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 0);
    static_assert(!star::HasSingleElementType<Range>);
  }
  {
    using Range = std::tuple<int>;
    static constexpr Range tup{1};

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 1);
    static_assert(star::get_at<0>(tup) == 1);
    static_assert(star::HasSingleElementType<Range>);
  }
  {
    using Range = std::tuple<int, float, double>;
    static constexpr Range tup{1, 2.0F, 3.0};

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(tup) == 2.0F);
    static_assert(!star::HasSingleElementType<Range>);
  }
  {
    using Range = std::vector<int>;
    static_assert(!star::IsStaticRange<Range>);
  }

  // constant
  {
    static constexpr auto con = star::constant<5>(1);
    using Range = std::decay_t<decltype(con)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 5);
    static_assert(star::get_at<4>(con) == 1);
  }

  // enumerate
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto enu = arr | star::enumerate<std::size_t>;
    using Range = std::decay_t<decltype(enu)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(enu) == std::make_pair(thes::static_value<std::size_t{3}>, 1));
    static_assert(!star::HasSingleElementType<Range>);
  }
  {
    static constexpr std::tuple<int, float, double> tup{1, 2.0F, 3.0};
    static constexpr auto enu = tup | star::enumerate<int>;
    using Range = std::decay_t<decltype(enu)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(enu) == std::make_pair(thes::static_value<1>, 2.0F));
    static_assert(!star::HasSingleElementType<Range>);
  }
  {
    static constexpr auto con = star::constant<5>(1);
    static constexpr auto enu = con | star::enumerate<int>;
    using Range = std::decay_t<decltype(enu)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 5);
    static_assert(star::get_at<4>(enu) == std::make_pair(thes::static_value<4>, 1));
  }

  // map_values
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map = arr | star::map_values([](auto v) { return 2 * v; });
    using Range = std::decay_t<decltype(map)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(map) == 2);
    static_assert(star::HasSingleElementType<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map =
      arr | star::map_values([](auto v) { return 2 * v; }) | star::enumerate<int>;
    using Range = std::decay_t<decltype(map)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(map) == std::make_pair(thes::static_value<3>, 2));
    static_assert(!star::HasSingleElementType<Range>);
  }
  {
    static constexpr std::tuple<int, float, double> tup{1, 2.0F, 3.0};
    static constexpr auto map = tup | star::map_values([](auto v) { return 2 * v; });
    using Range = std::decay_t<decltype(map)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 3);
    static_assert(star::get_at<1>(map) == 4.0F);
    static_assert(!star::HasSingleElementType<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr std::tuple<int, float, double, unsigned> tup{1, 2.0F, 3.0, 4U};
    static constexpr auto map =
      star::map_values([](auto v1, auto v2) { return v1 * int(v2); })(arr, tup);
    using Range = std::decay_t<decltype(map)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<1>(map) == 8.0F);
    static_assert(star::HasSingleElementType<Range>);
  }
  {
    static constexpr std::array arr{0, 4, 3, 1};
    static constexpr auto map =
      arr | (star::map_values([](auto v) { return 2 * v; }) | star::enumerate<int>);
    using Range = std::decay_t<decltype(map)>;

    static_assert(star::IsStaticRange<Range>);
    static_assert(star::size<Range> == 4);
    static_assert(star::get_at<3>(map) == std::make_pair(thes::static_value<3>, 2));
    static_assert(!star::HasSingleElementType<Range>);
  }
}
