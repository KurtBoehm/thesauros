#ifndef INCLUDE_THESAUROS_RANGES_ENUMERATE_HPP
#define INCLUDE_THESAUROS_RANGES_ENUMERATE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-map.hpp"
#include "thesauros/utility/integer-cast.hpp"

namespace thes {
template<typename TSize, typename TIter>
struct EnumerateRange {
  using Value = std::pair<TSize, decltype(*std::declval<TIter>())>;

private:
  struct IterProv {
    struct IterTypes : public iter_provider::ValueTypes<Value, std::ptrdiff_t> {
      using IterState = TIter;
    };

    static constexpr Value deref(const auto& self) {
      return Value{*thes::safe_cast<TSize>(self.it_ - self.begin_), *self.it_};
    }

    static constexpr TIter& state(auto& self) {
      return self.it_;
    }
    static constexpr const TIter& state(const auto& self) {
      return self.it_;
    }
  };

public:
  struct const_iterator : public IteratorFacade<const_iterator, iter_provider::Map<IterProv>> {
    friend IterProv;
    explicit constexpr const_iterator(TIter begin, TIter it) : begin_(begin), it_(std::move(it)) {}

  private:
    TIter begin_;
    TIter it_;
  };

  constexpr EnumerateRange(TIter begin, TIter end)
      : begin_(std::move(begin)), end_(std::move(end)) {}

  constexpr const_iterator begin() const {
    return const_iterator(begin_, begin_);
  }
  constexpr const_iterator end() const {
    return const_iterator(begin_, end_);
  }

private:
  TIter begin_;
  TIter end_;
};

template<typename TSize, typename TRange>
inline constexpr auto enumerate(TRange&& container) {
  using Iter = decltype(container.begin());
  return EnumerateRange<TSize, Iter>{container.begin(), container.end()};
}
template<typename TSize, typename TIter>
inline constexpr auto enumerate(TIter begin, TIter end) {
  return EnumerateRange<TSize, TIter>{std::move(begin), std::move(end)};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_ENUMERATE_HPP
