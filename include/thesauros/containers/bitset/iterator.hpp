#ifndef INCLUDE_THESAUROS_CONTAINERS_BITSET_ITERATOR_HPP
#define INCLUDE_THESAUROS_CONTAINERS_BITSET_ITERATOR_HPP

#include <cassert>
#include <climits>
#include <cstddef>

#include "thesauros/iterator.hpp"

namespace thes::detail {
struct BitsetIterStateProv {
  struct IterTypes : public iter_provider::ValueTypes<bool, std::ptrdiff_t> {
    using IterState = std::size_t;
  };

  static constexpr std::size_t& state(auto& self) {
    return self.idx_;
  }
  static constexpr const std::size_t& state(const auto& self) {
    return self.idx_;
  }
  static constexpr bool deref(const auto& self) {
    return self.self_.get(self.idx_);
  }
  static constexpr void test_if_cmp([[maybe_unused]] const auto& i1,
                                    [[maybe_unused]] const auto& i2) {
    assert(&i1.self_ == &i2.self_);
  }
};

template<typename TBitset>
struct BitsetIterator : public IteratorFacade<BitsetIterator<TBitset>,
                                              iter_provider::Map<detail::BitsetIterStateProv>> {
  friend detail::BitsetIterStateProv;
  constexpr BitsetIterator(std::size_t idx, const TBitset& self) : idx_(idx), self_(self) {}

private:
  std::size_t idx_;
  const TBitset& self_;
};
} // namespace thes::detail

#endif // INCLUDE_THESAUROS_CONTAINERS_BITSET_ITERATOR_HPP
