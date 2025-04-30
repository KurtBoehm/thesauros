// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <utility>
#include <variant>

#include "thesauros/utility.hpp"

namespace nsp1 {
using T1 = thes::TypeSeq<int, float>;
using T2 = thes::TypeSeq<long, double>;
using T3 = thes::FlatTypeSeq<thes::TypeSeq<T1, T2>>;

using V1 = std::variant<int, float>;
using V2 = std::variant<long, double>;
static_assert(std::same_as<thes::FlatTypeSeq<thes::TypeSeq<V1, V2>>,
                           thes::TypeSeq<std::variant<int, float>, std::variant<long, double>>>);
using V4 = thes::ConvertedTypeSeq<thes::TypeSeq<V1, V2>>;
static_assert(
  std::same_as<V4, thes::TypeSeq<thes::TypeSeq<thes::TypeSeq<int>, thes::TypeSeq<float>>,
                                 thes::TypeSeq<thes::TypeSeq<long>, thes::TypeSeq<double>>>>);
using V5 = thes::FlatTypeSeq<thes::TypeSeq<V4, V4>>;
static_assert(std::same_as<V5, thes::TypeSeq<int, float, long, double, int, float, long, double>>);

using Prod = thes::CartesianTypeSeq<thes::TypeSeq<int, float>, thes::TypeSeq<short, long, char>,
                                    thes::TypeSeq<double, long>>;
static_assert(
  std::same_as<
    Prod, thes::TypeSeq<thes::TypeSeq<int, short, double>, thes::TypeSeq<int, short, long>,
                        thes::TypeSeq<int, long, double>, thes::TypeSeq<int, long, long>,
                        thes::TypeSeq<int, char, double>, thes::TypeSeq<int, char, long>,
                        thes::TypeSeq<float, short, double>, thes::TypeSeq<float, short, long>,
                        thes::TypeSeq<float, long, double>, thes::TypeSeq<float, long, long>,
                        thes::TypeSeq<float, char, double>, thes::TypeSeq<float, char, long>>>);
} // namespace nsp1

struct Test1 {
  int a;

  constexpr explicit Test1(int p_a) : a{p_a} {}
  constexpr bool operator==(const Test1&) const = default;
};
struct Test2 {
  double a;

  constexpr explicit Test2(double p_a) : a{p_a} {}
  constexpr bool operator==(const Test2&) const = default;
};
struct Test3 {
  float a;

  constexpr explicit Test3(float p_a) : a{p_a} {}
  constexpr bool operator==(const Test3&) const = default;
};
using Test4 = std::variant<Test1, Test2>;
using Test5 = std::variant<Test1, Test3>;
using Test6 = std::variant<Test4, Test5>;

inline constexpr Test1 t1{1};
inline constexpr Test2 t2{2};
inline constexpr Test3 t3{3};
inline constexpr Test4 t4{std::in_place_type<Test1>, 4};
inline constexpr Test5 t5{std::in_place_type<Test1>, 5};
inline constexpr Test6 t6{std::in_place_type<Test4>, t4};

inline constexpr auto v1 = thes::fancy_visit([](auto v) { return v.a; }, t4);
inline constexpr auto v1a = std::get<int>(v1);
static_assert(v1a == 4);

inline constexpr auto v2 = thes::fancy_visit([](auto v) { return v.a; }, t5);
inline constexpr auto v2a = std::get<int>(v2);
static_assert(v2a == 5);

inline constexpr auto v3 = thes::fancy_visit([](auto v) { return v; }, t6);
static_assert(std::get<v3.index()>(v3) == t4);

inline constexpr auto v4 = thes::fancy_flat_visit(
  [](auto maker, auto var1) {
    return thes::fancy_visit_with_maker(
      maker,
      [&]<typename TVar2>(auto /*maker*/, TVar2 var2) {
        return maker(std::in_place_type<TVar2>, var2);
      },
      var1);
  },
  t6);

inline constexpr std::variant<float, int> in5{1.F};
inline constexpr decltype(auto) v5a =
  thes::fancy_visit([](auto& v) -> decltype(auto) { return v; }, in5);
inline constexpr decltype(auto) v5b = std::get<0>(v5a).get();
static_assert(v5b == 1.F);

int main() {}
