#ifndef INCLUDE_THESAUROS_UTILITY_NO_OP_HPP
#define INCLUDE_THESAUROS_UTILITY_NO_OP_HPP

namespace thes {
struct NoOp {
  constexpr void operator()() const noexcept {}
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_NO_OP_HPP
