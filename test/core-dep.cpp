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
// `macropolis`, `reflection`, `io`, `static-ranges`, `utility`, `resources`, `format`, and `test`.

#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

#include "thesauros/algorithms.hpp"
#include "thesauros/charconv.hpp"
#include "thesauros/concepts.hpp"
#include "thesauros/containers.hpp"
#include "thesauros/execution.hpp"
#include "thesauros/functional.hpp"
#include "thesauros/iterator.hpp" // IWYU pragma: keep
#include "thesauros/literals.hpp"
#include "thesauros/math.hpp"
#include "thesauros/memory.hpp"
#include "thesauros/quantity.hpp"
#include "thesauros/random.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/string.hpp"
#include "thesauros/types.hpp"

struct It : thes::StateIteratorFacade<thes::iter::ValueTypes<int, std::ptrdiff_t>> {
  friend thes::StateIteratorFacade<thes::iter::ValueTypes<int, std::ptrdiff_t>>;

  int v;

private:
  [[nodiscard]] int value() const {
    return v;
  }
  [[nodiscard]] auto& state(this auto& self) {
    return self.v;
  }
};

int main() {
  using namespace thes::literals;

  // Touch one entity per sub-library.
  static_assert(thes::star::index_to_position(1UZ, std::array{2UZ, 2UZ}) == std::array{0UZ, 1UZ});
  static_assert(thes::numeric_string(7).has_value());
  static_assert(thes::CompleteType<thes::StaticCapacityString<3>>);
  static_assert(thes::LimitedArray<int, 2>{1}.size() == 1);
  static_assert(thes::FixedThreadPool::max_thread_num > 512);
  static_assert(std::is_void_v<decltype(thes::NoOp<>{}())>);
  static_assert(std::random_access_iterator<It>);
  static_assert("1"_it == 1);
  static_assert(!thes::safe_cast<thes::u8>(256).is_valid());
  static_assert(std::same_as<std::allocator_traits<thes::HugePagesAllocator<int>>::pointer, int*>);
  static_assert(thes::Quantity<int, thes::unit::byte>{1024}.count() == 1024);
  static_assert(*thes::Lcg{1, 3, 7}.begin() == 1);
  static_assert(thes::views::indices(3).size() == 3);
  static_assert(thes::StaticString{"ab"}.size == 2);
  static_assert(thes::auto_tag<1> == 1);
}
