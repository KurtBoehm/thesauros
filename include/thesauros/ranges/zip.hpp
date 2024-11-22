#ifndef INCLUDE_THESAUROS_RANGES_ZIP_HPP
#define INCLUDE_THESAUROS_RANGES_ZIP_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>

#include "thesauros/iterator.hpp"
#include "thesauros/utility.hpp"

namespace thes {
template<typename... TRanges>
struct ZipRange {
  using Value = Tuple<decltype(*std::declval<typename std::decay_t<TRanges>::const_iterator>())...>;
  using Iterators = Tuple<typename std::decay_t<TRanges>::const_iterator...>;

private:
  struct IterProv {
    // TODO Replace ptrdiff_t?
    using IterTypes = iter_provider::ValueTypes<Value, std::ptrdiff_t>;

    static constexpr Value deref(auto& self) {
      return self.its_ | star::transform([](const auto& it) -> decltype(auto) { return *it; }) |
             star::to_tuple;
    }
    static constexpr void incr(auto& self) {
      self.its_ | star::for_each([](auto& it) { ++it; });
    }
    static constexpr bool eq(auto& its1, auto& its2) {
      assert(star::transform([](auto i1, auto i2) { return i1 == i2; }, its1.its_, its2.its_) |
             star::has_unique_value);
      return star::get_at<0>(its1.its_) == star::get_at<0>(its2.its_);
    }
  };

public:
  struct const_iterator : public IteratorFacade<const_iterator, IterProv> {
    friend IterProv;
    explicit constexpr const_iterator(Iterators&& iterators) : its_(std::move(iterators)) {}

  private:
    Iterators its_;
  };

  explicit constexpr ZipRange(TRanges&&... ranges) : ranges_{std::forward<TRanges>(ranges)...} {}

  constexpr const_iterator begin() const {
    return const_iterator(ranges_ | star::transform([](const auto& r) { return std::begin(r); }) |
                          star::to_tuple);
  }
  constexpr const_iterator end() const {
    return const_iterator(ranges_ | star::transform([](const auto& r) { return std::end(r); }) |
                          star::to_tuple);
  }

  constexpr auto size() const {
    assert(ranges_ | star::transform([](const auto& r) { return std::size(r); }) |
           star::has_unique_value);
    return std::size(star::get_at<0>(ranges_));
  }

private:
  Tuple<TRanges...> ranges_;
};

template<typename... TRanges>
constexpr auto zip(TRanges&&... ranges) {
  return ZipRange<TRanges...>{std::forward<TRanges>(ranges)...};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_ZIP_HPP
