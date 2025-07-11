// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <type_traits>

#include "thesauros/types/type-sequence.hpp"

template<typename T>
struct Filter : public std::is_integral<T> {};

int main() {
  {
    using Seq = thes::TypeSeq<int, float, double, long>;
    static_assert(std::same_as<thes::FilteredTypeSeq<Seq, Filter>, thes::TypeSeq<int, long>>);
    static_assert(thes::filter(Seq{}, [](auto t) {
                    return std::integral<typename decltype(t)::Type>;
                  }) == thes::TypeSeq<int, long>{});
  }
}
