// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// Compiled against `thesauros_core_dep` alone, i.e. with no external include directories on the
// command line at all. It therefore fails to build the moment one of these sub-libraries starts
// reaching for {fmt}, Boost.Preprocessor or any other dependency, which is exactly the layering
// the split dependencies in the top-level meson.build promise.
//
// The sub-libraries deliberately absent are the ones that do have external dependencies:
// `macropolis`, `reflection`, `io`, `static-ranges`, `utility`, `execution`, `resources`,
// `format` and `test`.

#include "thesauros/algorithms.hpp"
#include "thesauros/charconv.hpp"
#include "thesauros/concepts.hpp"
#include "thesauros/containers.hpp"
#include "thesauros/filesystem.hpp"
#include "thesauros/functional.hpp"
#include "thesauros/iterator.hpp"
#include "thesauros/literals.hpp"
#include "thesauros/math.hpp"
#include "thesauros/memory.hpp"
#include "thesauros/quantity.hpp"
#include "thesauros/random.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/string.hpp"
#include "thesauros/types.hpp"

int main() {
  // Touch one entity per sub-library so the includes cannot be optimized into nothing.
  static_assert(thes::views::indices(3).size() == 3);
  static_assert(thes::StaticString{"ab"}.size == 2);
  static_assert(thes::numeric_string(7).has_value());
  return 0;
}
