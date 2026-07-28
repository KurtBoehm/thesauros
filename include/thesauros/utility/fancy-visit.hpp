// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_FANCY_VISIT_HPP
#define INCLUDE_THESAUROS_UTILITY_FANCY_VISIT_HPP

#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "thesauros/types/type-sequence/operations.hpp"
#include "thesauros/types/type-sequence/type-sequence.hpp"
#include "thesauros/utility/unwrap.hpp"

namespace thes {
struct FancyVisitorIgnore {};
inline constexpr FancyVisitorIgnore fancy_visitor_ignore{};

template<bool RemoveIgnored, bool Flatten, bool WithMaker, typename Visitor, typename... Variants>
struct FancyVisitor {
  template<typename Raw, typename TDecayed>
  struct VariantHandlerImpl {
    using Tuple = TypeSeq<Raw>;
    using Base = std::remove_reference_t<Raw>;

    static constexpr auto pack(Raw& value)
    requires(std::is_lvalue_reference_v<Raw>)
    {
      using RefWrap = std::reference_wrapper<Base>;
      return std::variant<RefWrap>{RefWrap(value)};
    }

    static constexpr auto pack(Raw&& value)
    requires(!std::is_lvalue_reference_v<Raw>)
    {
      return std::variant<Raw>{std::move(value)};
    }
  };
  template<typename Raw, typename... Ts>
  struct VariantHandlerImpl<Raw, std::variant<Ts...>> {
    using Type = std::variant<Ts...>;
    static constexpr bool is_lvalue_ref = std::is_lvalue_reference_v<Raw>;
    static constexpr bool is_const = std::is_const_v<std::remove_reference_t<Raw>>;

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
  template<typename Raw>
  using VariantHandler = VariantHandlerImpl<Raw, std::decay_t<Raw>>;

  template<typename Seq>
  struct BareFunReturnType;
  template<typename... Args>
  struct BareFunReturnType<TypeSeq<Args...>> {
    using Type = decltype(std::declval<Visitor>()(unwrap(std::declval<Args>())...));
  };

  template<typename Seq>
  struct TaggedFunReturnType;
  template<typename... Args>
  struct TaggedFunReturnType<TypeSeq<Args...>> {
    using Type = decltype(std::declval<Visitor>()(
      []<typename T, typename... TInnerArgs>(std::in_place_type_t<T>, TInnerArgs&&... args) {
        return T{std::forward<TInnerArgs>(args)...};
      },
      unwrap(std::declval<Args>())...));
  };

  template<typename Seq>
  using FunReturnType =
    std::conditional_t<WithMaker, TaggedFunReturnType<Seq>, BareFunReturnType<Seq>>::Type;

  template<typename Seq>
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

  template<typename Seq>
  struct Maker;

  template<typename... Ts>
  struct Maker<TypeSeq<Ts...>> {
    using Return = std::variant<Ts...>;

    template<typename T, typename... Args>
    constexpr Return operator()(std::in_place_type_t<T> tag, Args&&... args) const {
      return Return{tag, std::forward<Args>(args)...};
    }
  };

  template<typename T>
  struct Maker<TypeSeq<T>> {
    using Return = T;

    template<typename... Args>
    constexpr Return operator()(std::in_place_type_t<T> /*tag*/, Args&&... args) const {
      return Return{std::forward<Args>(args)...};
    }
  };

  using Params = CartesianTypeSeq<typename VariantHandler<Variants>::Tuple...>;

  using RawReturnSeq = TransformedTypeSeq<Params, FunReturnType>;

  using BaseReturnSeq =
    std::conditional_t<Flatten, FlatTypeSeq<VariantTypeSeq<RawReturnSeq>>, RawReturnSeq>;

  using ReturnSeq = UniqueTypeSeq<FilteredTypeSeqBy<BaseReturnSeq, []<typename T>(TypeTag<T>) {
    return !std::same_as<T, FancyVisitorIgnore>;
  }>>;

  using Return = MakeReturn<ReturnSeq>::Type;

  static constexpr Maker<ReturnSeq> construct_in_place{};

  template<typename... Args>
  static constexpr decltype(auto) call(auto maker, Visitor&& visitor, Args&&... args) {
    if constexpr (WithMaker) {
      return visitor(maker, unwrap(std::forward<Args>(args))...);
    } else {
      return visitor(unwrap(std::forward<Args>(args))...);
    }
  }

  template<typename Maker, typename... Args>
  static constexpr bool ignore = requires(Maker maker, Visitor&& visitor, Args&&... args) {
    {
      call(maker, std::forward<Visitor>(visitor), std::forward<Args>(args)...)
    } -> std::convertible_to<FancyVisitorIgnore>;
  };

  template<typename Return, typename Maker>
  static constexpr decltype(auto) visit_impl(Maker maker, Visitor&& visitor, Variants&&... vars) {
    return std::visit(
      [&]<typename... Args>(Args&&... args) -> Return {
        if constexpr (RemoveIgnored && ignore<Maker, Args...>) {
          call(maker, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
          throw std::invalid_argument("The visitor failed!");
        } else {
          return call(maker, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
        }
      },
      VariantHandler<Variants>::pack(std::forward<Variants>(vars))...);
  }

  static constexpr decltype(auto) visit(Visitor&& visitor, Variants&&... vars) {
    return visit_impl<Return>(construct_in_place, std::forward<Visitor>(visitor),
                              std::forward<Variants>(vars)...);
  }

  template<typename Maker>
  static constexpr decltype(auto) visit_with_maker(Maker maker, Visitor&& visitor,
                                                   Variants&&... vars) {
    if constexpr (requires { typename Maker::Return; }) {
      return visit_impl<typename Maker::Return>(maker, std::forward<Visitor>(visitor),
                                                std::forward<Variants>(vars)...);
    } else {
      return visit(std::forward<Visitor>(visitor), std::forward<Variants>(vars)...);
    }
  }
};

template<typename Visitor, typename... Vars>
constexpr decltype(auto) fancy_visit(Visitor&& visitor, Vars&&... vars) {
  return FancyVisitor<false, false, false, Visitor, Vars...>::visit(std::forward<Visitor>(visitor),
                                                                    std::forward<Vars>(vars)...);
}

template<typename Visitor, typename... Vars>
constexpr decltype(auto) fancy_filter_visit(Visitor&& visitor, Vars&&... vars) {
  return FancyVisitor<true, false, false, Visitor, Vars...>::visit(std::forward<Visitor>(visitor),
                                                                   std::forward<Vars>(vars)...);
}

template<typename Visitor, typename... Vars>
constexpr decltype(auto) fancy_maker_visit(Visitor&& visitor, Vars&&... vars) {
  return FancyVisitor<true, false, true, Visitor, Vars...>::visit(std::forward<Visitor>(visitor),
                                                                  std::forward<Vars>(vars)...);
}

template<typename Visitor, typename... Vars>
constexpr decltype(auto) fancy_flat_visit(Visitor&& visitor, Vars&&... vars) {
  return FancyVisitor<true, true, true, Visitor, Vars...>::visit(std::forward<Visitor>(visitor),
                                                                 std::forward<Vars>(vars)...);
}

template<typename Maker, typename Visitor, typename... Vars>
constexpr decltype(auto) fancy_visit_with_maker(Maker maker, Visitor&& visitor, Vars&&... vars) {
  return FancyVisitor<true, true, true, Visitor, Vars...>::visit_with_maker(
    maker, std::forward<Visitor>(visitor), std::forward<Vars>(vars)...);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_FANCY_VISIT_HPP
