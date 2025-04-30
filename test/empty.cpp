// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <tuple>

#include "thesauros/utility/empty.hpp"

int main() {
  static constexpr thes::Empty empty{};
  static_assert(!empty.has_value());

  static_assert(thes::apply_empty([](int i) { return i; }, std::make_tuple(1)) == 1);
  static_assert(thes::apply_empty([](int /*i*/) { return; }, std::make_tuple(1)) == empty);
}
