// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONCEPTS_NOTHROW_HPP
#define INCLUDE_THESAUROS_CONCEPTS_NOTHROW_HPP

#include <concepts>
#include <type_traits>

namespace thes {
/** Whether `Fun` can be invoked with `Args` and is declared not to throw when doing so. */
template<typename Fun, typename... Args>
concept NothrowInvocable =
  std::invocable<Fun, Args...> && std::is_nothrow_invocable_v<Fun, Args...>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONCEPTS_NOTHROW_HPP
