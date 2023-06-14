#ifndef INCLUDE_THESAUROS_UTILITY_NO_OP_HPP
#define INCLUDE_THESAUROS_UTILITY_NO_OP_HPP

#include <type_traits>

namespace thes {
struct NoOp {
  constexpr void operator()() const noexcept {}
};
template<typename TOp>
struct AnyNoOpTrait : public std::false_type {};
template<>
struct AnyNoOpTrait<NoOp> : public std::true_type {};
template<typename TOp>
concept AnyNoOp = AnyNoOpTrait<TOp>::value;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_NO_OP_HPP
