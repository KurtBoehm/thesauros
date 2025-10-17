// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <vector>

#include "thesauros/format.hpp"
#include "thesauros/quantity.hpp"
#include "thesauros/resources.hpp"

namespace {
using Dur = thes::ResourceUsage::Duration;
auto convert_time(Dur q) {
  return quantity_cast<thes::Quantity<double, thes::unit::second>>(q);
}
} // namespace

int main() {
  using Clock = std::chrono::steady_clock;
  const auto t1 = Clock::now();

  static constexpr thes::Quantity<int, thes::unit::metre> sc1(2);
  static constexpr auto v =
    quantity_cast<thes::Quantity<float, thes::unit::kilometre>>(sc1).count();
  static_assert(v == 2e-3F);

  std::vector<double> data(1U << 26U, 1);
  for (std::size_t i = 0; i < 1; ++i) {
    std::ranges::generate(data, std::rand);
  }

  thes::ResourceUsage usage{};
  const auto rawmaxrss = usage.max_memory();
  const auto binmaxrss =
    thes::quantity_cast<thes::Quantity<double, thes::unit::gibibyte>>(rawmaxrss);
  const auto decmaxrss =
    thes::quantity_cast<thes::Quantity<double, thes::unit::gigabyte>>(rawmaxrss);
  fmt::print("maxrss: {}, {}\n", binmaxrss, decmaxrss);
  fmt::print("user time: {}\n", convert_time(usage.user_time()));
  fmt::print("user time: {}\n", split_time(usage.user_time()));
  fmt::print("system time: {}\n", convert_time(usage.system_time()));
  fmt::print("overall time: {}\n", convert_time(usage.user_time() + usage.system_time()));

  const auto t2 = Clock::now();
  const auto q =
    quantity_cast<thes::Quantity<double, thes::unit::second>>(thes::duration_quantity(t2 - t1));
  fmt::print("time: {}\n", q);
  fmt::print("time: {}\n", split_time(q));
}
