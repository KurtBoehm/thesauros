#ifndef INCLUDE_THESAUROS_UTILITY_FANCY_VISIT_HPP
#define INCLUDE_THESAUROS_UTILITY_FANCY_VISIT_HPP

#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "thesauros/types/type-sequence.hpp"
#include "thesauros/utility/unwrap.hpp"

namespace thes {
struct FancyVisitorIgnore {};
inline constexpr FancyVisitorIgnore fancy_visitor_ignore{};

template<bool tRemoveIgnored, bool tFlatten, bool tWithMaker, typename TVisitor,
         typename... TVariants>
struct FancyVisitor {
  template<typename TRaw, typename TDecayed>
  struct VariantHandlerImpl {
    using Tuple = TypeSeq<TRaw>;
    using Base = std::remove_reference_t<TRaw>;

    static constexpr auto pack(TRaw& value)
    requires(std::is_lvalue_reference_v<TRaw>)
    {
      using RefWrap = std::reference_wrapper<Base>;
      return std::variant<RefWrap>{RefWrap(value)};
    }

    static constexpr auto pack(TRaw&& value)
    requires(!std::is_lvalue_reference_v<TRaw>)
    {
      return std::variant<TRaw>{std::move(value)};
    }
  };
  template<typename TRaw, typename... Ts>
  struct VariantHandlerImpl<TRaw, std::variant<Ts...>> {
    using Type = std::variant<Ts...>;
    static constexpr bool is_lvalue_ref = std::is_lvalue_reference_v<TRaw>;
    static constexpr bool is_const = std::is_const_v<std::remove_reference_t<TRaw>>;

    template<typename T>
    using Constant = std::conditional_t<is_const, const T, T>;
    template<typename T>
    using Transformed = std::conditional_t<is_lvalue_ref, Constant<T>&, Constant<T>>;

    using Tuple = TypeSeq<Transformed<Ts>...>;

    template<typename TVar>
    requires(std::same_as<std::decay_t<TVar>, Type>)
    static constexpr TVar&& pack(TVar&& value) {
      return std::forward<TVar>(value);
    }
  };
  template<typename TRaw>
  using VariantHandler = VariantHandlerImpl<TRaw, std::decay_t<TRaw>>;

  template<typename TSeq>
  struct BareFunReturnType;
  template<typename... TArgs>
  struct BareFunReturnType<TypeSeq<TArgs...>> {
    using Type = decltype(std::declval<TVisitor>()(unwrap(std::declval<TArgs>())...));
  };

  template<typename TSeq>
  struct TaggedFunReturnType;
  template<typename... TArgs>
  struct TaggedFunReturnType<TypeSeq<TArgs...>> {
    using Type = decltype(std::declval<TVisitor>()(
      []<typename T, typename... TInnerArgs>(std::in_place_type_t<T>, TInnerArgs&&... args) {
        return T{std::forward<TInnerArgs>(args)...};
      },
      unwrap(std::declval<TArgs>())...));
  };

  template<typename TSeq>
  using FunReturnType =
    std::conditional_t<tWithMaker, TaggedFunReturnType<TSeq>, BareFunReturnType<TSeq>>::Type;

  template<typename TSeq>
  struct MakeReturn;
  template<typename T>
  struct MakeReturn<TypeSeq<T>> {
    using Type = T;
  };
  template<typename... Ts>
  struct MakeReturn<TypeSeq<Ts...>> {
    template<typename T>
    using Transformed = std::conditional_t<std::is_reference_v<T>,
                                           std::reference_wrapper<std::remove_reference_t<T>>, T>;
    using Type = std::variant<Transformed<Ts>...>;
  };

  template<typename TSeq>
  struct Maker;

  template<typename... Ts>
  struct Maker<TypeSeq<Ts...>> {
    using Return = std::variant<Ts...>;

    template<typename T, typename... TArgs>
    constexpr Return operator()(std::in_place_type_t<T> tag, TArgs&&... args) const {
      return Return{tag, std::forward<TArgs>(args)...};
    }
  };

  template<typename T>
  struct Maker<TypeSeq<T>> {
    using Return = T;

    template<typename... TArgs>
    constexpr Return operator()(std::in_place_type_t<T> /*tag*/, TArgs&&... args) const {
      return Return{std::forward<TArgs>(args)...};
    }
  };

  using Params = CartesianTypeSeq<typename VariantHandler<TVariants>::Tuple...>;

  using RawReturnSeq = TransformedTypeSeq<Params, FunReturnType>;

  using BaseReturnSeq =
    std::conditional_t<tFlatten, FlatTypeSeq<ConvertedTypeSeq<RawReturnSeq>>, RawReturnSeq>;

  template<typename T>
  struct ReturnFilter : std::bool_constant<!std::is_same_v<T, FancyVisitorIgnore>> {};
  using ReturnSeq = UniqueTypeSeq<FilteredTypeSeq<BaseReturnSeq, ReturnFilter>>;

  using Return = MakeReturn<ReturnSeq>::Type;

  static constexpr Maker<ReturnSeq> construct_in_place{};

  template<typename... TArgs>
  static constexpr decltype(auto) call(auto maker, TVisitor&& visitor, TArgs&&... args) {
    if constexpr (tWithMaker) {
      return visitor(maker, unwrap(std::forward<TArgs>(args))...);
    } else {
      return visitor(unwrap(std::forward<TArgs>(args))...);
    }
  }

  template<typename TMaker, typename... TArgs>
  static constexpr bool ignore = requires(TMaker maker, TVisitor&& visitor, TArgs&&... args) {
    {
      call(maker, std::forward<TVisitor>(visitor), std::forward<TArgs>(args)...)
    } -> std::convertible_to<FancyVisitorIgnore>;
  };

  template<typename TReturn, typename TMaker>
  static constexpr decltype(auto) visit_impl(TMaker maker, TVisitor&& visitor,
                                             TVariants&&... vars) {
    return std::visit(
      [&]<typename... TArgs>(TArgs&&... args) -> TReturn {
        if constexpr (tRemoveIgnored && ignore<TMaker, TArgs...>) {
          call(maker, std::forward<TVisitor>(visitor), std::forward<TArgs>(args)...);
          throw std::invalid_argument("The visitor failed!");
        } else {
          return call(maker, std::forward<TVisitor>(visitor), std::forward<TArgs>(args)...);
        }
      },
      VariantHandler<TVariants>::pack(std::forward<TVariants>(vars))...);
  }

  static constexpr decltype(auto) visit(TVisitor&& visitor, TVariants&&... vars) {
    return visit_impl<Return>(construct_in_place, std::forward<TVisitor>(visitor),
                              std::forward<TVariants>(vars)...);
  }

  template<typename TMaker>
  static constexpr decltype(auto) visit_with_maker(TMaker maker, TVisitor&& visitor,
                                                   TVariants&&... vars) {
    if constexpr (requires { typename TMaker::Return; }) {
      return visit_impl<typename TMaker::Return>(maker, std::forward<TVisitor>(visitor),
                                                 std::forward<TVariants>(vars)...);
    } else {
      return visit(std::forward<TVisitor>(visitor), std::forward<TVariants>(vars)...);
    }
  }
};

template<typename TVisitor, typename... TVars>
inline constexpr decltype(auto) fancy_visit(TVisitor&& visitor, TVars&&... vars) {
  return FancyVisitor<false, false, false, TVisitor, TVars...>::visit(
    std::forward<TVisitor>(visitor), std::forward<TVars>(vars)...);
}

template<typename TVisitor, typename... TVars>
inline constexpr decltype(auto) fancy_filter_visit(TVisitor&& visitor, TVars&&... vars) {
  return FancyVisitor<true, false, false, TVisitor, TVars...>::visit(
    std::forward<TVisitor>(visitor), std::forward<TVars>(vars)...);
}

template<typename TVisitor, typename... TVars>
inline constexpr decltype(auto) fancy_maker_visit(TVisitor&& visitor, TVars&&... vars) {
  return FancyVisitor<true, false, true, TVisitor, TVars...>::visit(std::forward<TVisitor>(visitor),
                                                                    std::forward<TVars>(vars)...);
}

template<typename TVisitor, typename... TVars>
inline constexpr decltype(auto) fancy_flat_visit(TVisitor&& visitor, TVars&&... vars) {
  return FancyVisitor<true, true, true, TVisitor, TVars...>::visit(std::forward<TVisitor>(visitor),
                                                                   std::forward<TVars>(vars)...);
}

template<typename TMaker, typename TVisitor, typename... TVars>
inline constexpr decltype(auto) fancy_visit_with_maker(TMaker maker, TVisitor&& visitor,
                                                       TVars&&... vars) {
  return FancyVisitor<true, true, true, TVisitor, TVars...>::visit_with_maker(
    maker, std::forward<TVisitor>(visitor), std::forward<TVars>(vars)...);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_FANCY_VISIT_HPP
