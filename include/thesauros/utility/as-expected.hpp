#ifndef INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP
#define INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP

#include "tl/expected.hpp"

namespace thes {
inline constexpr tl::expected<void, int> as_expected(int ret) {
  using Out = tl::expected<void, int>;

  Out out{};
  if (ret != 0) {
    out = tl::unexpected(ret);
  }

  return out;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP
