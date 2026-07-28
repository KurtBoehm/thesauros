// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MATH_FACTORIZATION_HPP
#define INCLUDE_THESAUROS_MATH_FACTORIZATION_HPP

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "thesauros/math/integer-cast.hpp"

namespace thes {
namespace detail::factorize {
template<std::unsigned_integral T, typename Map>
struct MapFactorization {
  Map factorization{};

  struct IterativeEmplacer {
    T k;
    T cnt{};
    Map& factorization;

    IterativeEmplacer(T factor, Map& factors) : k{factor}, factorization(factors) {}
    ~IterativeEmplacer() {
      if (cnt > 0) {
        factorization.emplace(k, cnt);
      }
    }

    IterativeEmplacer& operator++() {
      ++cnt;
      return *this;
    }
  };

  void emplace(T k, T v) {
    factorization.emplace(k, v);
  }
  IterativeEmplacer iterative_emplacer(T k) {
    return {k, factorization};
  }
};
template<std::unsigned_integral T, typename Arr>
struct FlatFactorization {
  Arr factorization{};

  struct IterativeEmplacer {
    T k;
    Arr& factorization;

    IterativeEmplacer(T factor, Arr& factors) : k{factor}, factorization(factors) {}

    IterativeEmplacer& operator++() {
      factorization.push_back(k);
      return *this;
    }
  };

  void emplace(T k, T v) {
    for (; v > 0; --v) {
      factorization.push_back(k);
    }
  }
  IterativeEmplacer iterative_emplacer(T k) {
    return {k, factorization};
  }
};
template<std::unsigned_integral T>
inline auto factorize(T n, auto&& factorization) {
  assert(n > 0);

  if (const auto two_num = *thes::safe_cast<T>(std::countr_zero(n)); two_num > 0) {
    n >>= two_num;
    factorization.emplace(2, two_num);
  }

  for (T d : {T{3}, T{5}}) {
    auto iter = factorization.iterative_emplacer(d);
    for (; n % d == 0; ++iter) {
      n /= d;
    }
  }

  static constexpr std::array<T, 8> increments{4, 2, 4, 2, 4, 6, 2, 6};
  std::size_t i = 0;
  for (T d = 7; d * d <= n; d += increments[i], i = (i + 1) % 8) {
    auto iter = factorization.iterative_emplacer(d);
    for (; n % d == 0; ++iter) {
      n /= d;
    }
  }
  if (n > 1) {
    factorization.emplace(n, 1);
  }
  return std::move(factorization.factorization);
}
} // namespace detail::factorize

template<std::unsigned_integral T, typename Map = std::unordered_map<T, T>>
inline Map factorize_map(T n) {
  return detail::factorize::factorize(n, detail::factorize::MapFactorization<T, Map>{});
}
/**
 * The prime factors of `n`, with multiplicity, in ascending order, collected into `Arr`, which
 * only has to support `push_back`.
 */
template<std::unsigned_integral T, typename Arr = std::vector<T>>
inline Arr factorize_flat(T n) {
  return detail::factorize::factorize(n, detail::factorize::FlatFactorization<T, Arr>{});
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_FACTORIZATION_HPP
