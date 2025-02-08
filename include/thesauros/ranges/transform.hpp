#ifndef INCLUDE_THESAUROS_RANGES_TRANSFORM_HPP
#define INCLUDE_THESAUROS_RANGES_TRANSFORM_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-map.hpp"
#include "thesauros/utility/integer-cast.hpp"

namespace thes {
template<typename TOp, typename TIter>
struct TransformRange {
  using Value = decltype(std::declval<TOp>()(*std::declval<TIter>()));

private:
  struct IterProv {
    struct IterTypes : public iter_provider::ValueTypes<Value, std::ptrdiff_t> {
      using IterState = TIter;
    };

    static constexpr Value deref(auto& self) {
      return self.op_(*self.it_);
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
    constexpr const_iterator() = default;
    explicit constexpr const_iterator(TOp op, TIter it) : op_(op), it_(std::move(it)) {}

  private:
    TOp op_{};
    TIter it_{};
  };

  constexpr TransformRange(TOp&& op, TIter begin, TIter end)
      : op_(std::forward<TOp>(op)), begin_(std::move(begin)), end_(std::move(end)) {}

  constexpr const_iterator begin() const {
    return const_iterator(op_, begin_);
  }
  constexpr const_iterator end() const {
    return const_iterator(op_, end_);
  }

  constexpr TIter raw_begin() const {
    return begin_;
  }
  constexpr TIter raw_end() const {
    return end_;
  }

  decltype(auto) operator[](const auto& idx) const
  requires(requires { this->raw_begin()[idx]; })
  {
    return op_(begin_[idx]);
  }
  decltype(auto) operator[](const auto& idx)
  requires(requires { this->raw_begin()[idx]; })
  {
    return op_(begin_[idx]);
  }

  auto size() const
  requires(requires { this->raw_end() - this->raw_begin(); })
  {
    assert(end_ - begin_ >= 0);
    using Signed = decltype(end_ - begin_);
    return *safe_cast<std::make_unsigned_t<Signed>>(end_ - begin_);
  }

private:
  TOp op_;
  TIter begin_;
  TIter end_;
};
template<typename TOp, typename TIter>
TransformRange(TOp&&, TIter, TIter) -> TransformRange<TOp, TIter>;

template<typename TOp, typename TRange>
inline constexpr auto transform_range(TOp op, TRange&& container) {
  return TransformRange{std::forward<TOp>(op), container.begin(), container.end()};
}
template<typename TOp, typename TIter>
inline constexpr auto transform_range(TOp op, TIter begin, TIter end) {
  return TransformRange{std::forward<TOp>(op), std::move(begin), std::move(end)};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_TRANSFORM_HPP
