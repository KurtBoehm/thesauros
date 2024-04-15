#ifndef INCLUDE_THESAUROS_IO_DELIMITER_HPP
#define INCLUDE_THESAUROS_IO_DELIMITER_HPP

#include <ostream>
#include <string_view>
#include <utility>

namespace thes {
struct Delimiter {
  using Raw = std::string_view;

  explicit constexpr Delimiter(Raw str) : str_(std::forward<Raw>(str)) {}

  template<typename TIt>
  TIt output(TIt it, char sep) const {
    return output_impl(it, [&] { *it++ = sep; });
  }

  auto output(auto it) const {
    return output_impl(it, [] {});
  }

  friend std::ostream& operator<<(std::ostream& s, const Delimiter& delim) {
    if (delim.first_) {
      delim.first_ = false;
    } else {
      s << delim.str_;
    }
    return s;
  }

private:
  template<typename TIt>
  TIt output_impl(TIt it, auto op) const {
    if (first_) {
      first_ = false;
    } else {
      it = std::copy(str_.begin(), str_.end(), it);
      op();
    }
    return it;
  }

  Raw str_;
  mutable bool first_ = true;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_DELIMITER_HPP
