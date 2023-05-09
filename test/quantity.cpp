#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "thesauros/quantity.hpp"
#include "thesauros/utility.hpp"

auto convert_time(thes::Quantity<long, thes::unit::microsecond> q) {
  return quantity_cast<thes::Quantity<double, thes::unit::second>>(q);
}

int main() {
  using Clock = std::chrono::steady_clock;
  const auto t1 = Clock::now();

  static constexpr thes::Quantity<int, thes::unit::metre> sc1(2);
  static constexpr auto v =
    quantity_cast<thes::Quantity<float, thes::unit::kilometre>>(sc1).count();
  static_assert(v == 2e-3F);

  std::vector<double> data(1U << 27U, 1);
  for (std::size_t i = 0; i < 1; ++i) {
    std::generate(data.begin(), data.end(), std::rand);
  }

  thes::ResourceUsage usage{};
  const auto rawmaxrss = usage.max_memory();
  const auto binmaxrss =
    thes::quantity_cast<thes::Quantity<double, thes::unit::gibibyte>>(rawmaxrss);
  const auto decmaxrss =
    thes::quantity_cast<thes::Quantity<double, thes::unit::gigabyte>>(rawmaxrss);
  std::cout << "maxrss: " << binmaxrss << ", " << decmaxrss << std::endl;
  std::cout << "user time: " << convert_time(usage.user_time()) << std::endl;
  std::cout << "user time: " << split_time(usage.user_time()) << std::endl;
  std::cout << "system time: " << convert_time(usage.system_time()) << std::endl;
  std::cout << "overall time: " << convert_time(usage.user_time() + usage.system_time())
            << std::endl;

  const auto t2 = Clock::now();
  const auto q =
    quantity_cast<thes::Quantity<double, thes::unit::second>>(thes::duration_quantity(t2 - t1));
  std::cout << "time: " << q << std::endl;
  std::cout << "time: " << split_time(q) << std::endl;
}
