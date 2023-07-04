#include "thesauros/utility.hpp"

static constexpr thes::Optional<double> o1{2};
static_assert(o1.and_then([](auto v) { return thes::Optional{v * 2}; }).value() == 4);
static_assert(o1.transform([](auto v) { return 3 * v; }).value() == 6);
static_assert(o1.or_else([] { return thes::Optional{4.0}; }).value() == 2);

static constexpr thes::Optional<double> o2{};
static_assert(!o2.and_then([](auto v) { return thes::Optional{v * 2}; }).has_value());
static_assert(!o2.transform([](auto v) { return 3 * v; }).has_value());
static_assert(o2.or_else([] { return thes::Optional{4.0}; }).value() == 4);

int main() {}
