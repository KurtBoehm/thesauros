// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_BITSET_ITERATOR_HPP
#define INCLUDE_THESAUROS_CONTAINERS_BITSET_ITERATOR_HPP

#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "thesauros/iterator/facade.hpp"
#include "thesauros/iterator/state-facade.hpp"

namespace thes::detail {
/**
 * An iterator over `Bitset`. If `IsConst` is `false`, dereferencing yields `Bitset::MutBitRef`,
 * allowing bits to be assigned to through the iterator; otherwise it yields `bool` by value.
 */
template<typename Bitset, bool IsConst>
struct BitsetIterator
    : public StateIteratorFacade<iter::ValueTypes<
        std::conditional_t<IsConst, bool, typename Bitset::MutBitRef>, std::ptrdiff_t>> {
  using Facade = StateIteratorFacade<iter::ValueTypes<
    std::conditional_t<IsConst, bool, typename Bitset::MutBitRef>, std::ptrdiff_t>>;
  friend Facade;

  using Container = std::conditional_t<IsConst, const Bitset, Bitset>;

  constexpr BitsetIterator() = default;
  constexpr BitsetIterator(std::size_t idx, Container& self)
      : idx_(idx), self_(std::addressof(self)) {}

private:
  [[nodiscard]] constexpr decltype(auto) value() const {
    return (*self_)[idx_];
  }
  [[nodiscard]] constexpr auto& state(this auto& self) {
    return self.idx_;
  }
  constexpr void test_if_cmp([[maybe_unused]] const BitsetIterator& other) const {
    assert(self_ == other.self_);
  }

  std::size_t idx_{};
  Container* self_{};
};
} // namespace thes::detail

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_ITERATOR_HPP
