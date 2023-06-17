#include "thesauros/math.hpp"

static_assert(thes::add_max<unsigned char>(1, 4, 3) == 3);
static_assert(thes::add_max<unsigned char>(1, 2, 4) == 3);
static_assert(thes::add_max<unsigned char>(128, 128, 254) == 254);

static_assert(thes::sub_min<unsigned>(5, 3, 2) == 2);
static_assert(thes::sub_min<unsigned>(5, 7, 2) == 2);

int main() {}
