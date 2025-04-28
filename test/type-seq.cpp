#include <concepts>
#include <type_traits>

#include "thesauros/types/type-sequence/operations.hpp"
#include "thesauros/utility.hpp"

template<typename T>
struct Filter : public std::is_integral<T> {};

int main() {
  {
    using Seq = thes::TypeSeq<int, float, double, long>;
    static_assert(std::same_as<thes::FilteredTypeSeq<Seq, Filter>, thes::TypeSeq<int, long>>);
    static_assert(thes::filter(Seq{}, [](auto t) {
                    return std::integral<typename decltype(t)::Type>;
                  }) == thes::TypeSeq<int, long>{});
  }
}
