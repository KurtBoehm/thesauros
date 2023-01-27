#include <concepts>

#include "thesauros/utility/value-sequence.hpp"

int main() {
  {
    using Seq1 = thes::AutoValueSequence<1, 2>;
    using Seq2 = Seq1::Prepend<0>;
    using Seq3 = Seq2::Append<3>;
    static_assert(std::same_as<Seq3, thes::AutoValueSequence<0, 1, 2, 3>>);
    constexpr auto val1 = Seq3::get_at<3>;
    static_assert(val1 == 3);
  }
  {
    using Seq1 = thes::ValueSequence<double>;
    static_assert(Seq1::size == 0);
  }
  {
    using Seq1 = thes::AutoMakeIntegerSequence<0, 4>;
    static_assert(std::same_as<Seq1, thes::AutoValueSequence<0, 1, 2, 3>>);
    static_assert(Seq1::contains<3>);
    static_assert(!Seq1::contains<4>);
  }
  {
    using Seq1 = thes::AutoValueSequence<3, 1, 2, 0>;
    using Seq2 = Seq1::ExceptValues<1>;
    static_assert(std::same_as<Seq2, thes::AutoValueSequence<3, 2, 0>>);
  }
  {
    using Seq1 = thes::ValueSequence<int>;
    using Seq2 = Seq1::ExceptSequence<thes::AutoValueSequence<2>>;
    static_assert(std::same_as<Seq2, thes::ValueSequence<int>>);
  }
}
