// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include "thesauros/static-ranges/type-seq.hpp"
#include "thesauros/types/tuple.hpp"
#include "thesauros/types/type-sequence.hpp"

template<typename T>
struct Filter : public std::is_integral<T> {};
template<typename T>
using AddPointer = T*;

int main() {
  {
    using Seq = thes::TypeSeq<int, float, double, long>;
    static_assert(
      std::same_as<thes::IndexFilteredTypeSeq<Seq, std::array{0, 3}>, thes::TypeSeq<int, long>>);
    static_assert(
      std::same_as<thes::IndexFilteredTypeSeq<Seq, thes::Tuple{0, 3}>, thes::TypeSeq<int, long>>);
  }
  {
    using Seq = thes::TypeSeq<int, float, double, long>;
    static_assert(std::same_as<thes::FilteredTypeSeq<Seq, Filter>, thes::TypeSeq<int, long>>);
    static_assert(thes::filter(Seq{}, [](auto t) {
                    return std::integral<typename decltype(t)::Type>;
                  }) == thes::TypeSeq<int, long>{});
    // The `Fun` NTTP overload needs no `decltype` to name the lambda’s type.
    static_assert(thes::filter<[](auto t) { return std::integral<typename decltype(t)::Type>; }>(
                    Seq{}) == thes::TypeSeq<int, long>{});
  }
  {
    using Seq = thes::TypeSeq<int, float, double, long>;
    using Ptrs = thes::TypeSeq<int*, float*, double*, long*>;
    static_assert(std::same_as<thes::TransformedTypeSeq<Seq, AddPointer>, Ptrs>);
    static_assert(thes::transform(Seq{}, []<typename T>(thes::TypeTag<T>) {
                    return thes::type_tag<T*>;
                  }) == Ptrs{});
    static_assert(thes::transform<[]<typename T>(thes::TypeTag<T>) { return thes::type_tag<T*>; }>(
                    Seq{}) == Ptrs{});
    static_assert(
      std::same_as<thes::TransformedTypeSeqBy<
                     Seq, []<typename T>(thes::TypeTag<T>) { return thes::type_tag<T*>; }>,
                   Ptrs>);
  }
  {
    using Seq = thes::TypeSeq<std::uint8_t, std::uint16_t, std::uint32_t>;
    constexpr auto combine = []<typename Acc, typename T>(Acc, thes::TypeTag<T>) {
      return thes::auto_tag<Acc::value + sizeof(T)>;
    };
    static_assert(thes::reduce(Seq{}, thes::auto_tag<std::size_t{0}>, combine).value == 7);
    static_assert(thes::reduce<combine>(Seq{}, thes::auto_tag<std::size_t{0}>).value == 7);
  }
  {
    // `T` may also be taken as an explicit template parameter instead of a `TypeTag` argument.
    using Seq = thes::TypeSeq<int, float, double, long>;
    using Ptrs = thes::TypeSeq<int*, float*, double*, long*>;

    static_assert(thes::transform(Seq{}, []<typename T> { return thes::type_tag<T*>; }) == Ptrs{});
    static_assert(thes::transform<[]<typename T> { return thes::type_tag<T*>; }>(Seq{}) == Ptrs{});

    static_assert(thes::filter(Seq{}, []<typename T> { return std::integral<T>; }) ==
                  thes::TypeSeq<int, long>{});
    static_assert(thes::filter<[]<typename T> { return std::integral<T>; }>(Seq{}) ==
                  thes::TypeSeq<int, long>{});

    using SizeSeq = thes::TypeSeq<std::uint8_t, std::uint16_t, std::uint32_t>;
    constexpr auto combine = []<typename T>(auto acc) {
      return thes::auto_tag<decltype(acc)::value + sizeof(T)>;
    };
    static_assert(thes::reduce(SizeSeq{}, thes::auto_tag<std::size_t{0}>, combine).value == 7);
    static_assert(thes::reduce<combine>(SizeSeq{}, thes::auto_tag<std::size_t{0}>).value == 7);
  }
  {
    static_assert(
      std::same_as<thes::StaticRangeTypeSeq<std::array<int, 3>>, thes::TypeSeq<int, int, int>>);
    static_assert(std::same_as<thes::StaticRangeTypeSeq<std::tuple<int, float, double>>,
                               thes::TypeSeq<int, float, double>>);
    static_assert(std::same_as<thes::StaticRangeTypeSeq<thes::Tuple<int, float, double>>,
                               thes::TypeSeq<int, float, double>>);

    constexpr std::array<int, 3> arr{1, 2, 3};
    static_assert(thes::static_range_type_seq(arr) == thes::TypeSeq<int, int, int>{});
  }
}
