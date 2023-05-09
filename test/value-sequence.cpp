#include <concepts>

#include "thesauros/utility/value-sequence.hpp"

int main() {
  {
    using Seq1 = thes::AutoSequence<1, 2>;
    using Seq2 = Seq1::Prepended<0>;
    using Seq3 = Seq2::Appended<3>;
    static_assert(std::same_as<Seq3, thes::AutoSequence<0, 1, 2, 3>>);
    constexpr auto val1 = Seq3::at<3>;
    static_assert(val1 == 3);
  }
  {
    using Seq1 = thes::ValueSequence<double>;
    static_assert(Seq1::size == 0);
  }
  {
    using Seq1 = thes::MakeAutoIntegerSequence<0, 4>;
    static_assert(std::same_as<Seq1, thes::AutoSequence<0, 1, 2, 3>>);
    static_assert(Seq1::contains<3>);
    static_assert(!Seq1::contains<4>);
  }
  {
    using Seq1 = thes::AutoSequence<3, 1, 2, 0>;
    using Seq2 = Seq1::ExceptValues<1>;
    static_assert(std::same_as<Seq2, thes::AutoSequence<3, 2, 0>>);
  }
  {
    using Seq1 = thes::ValueSequence<int>;
    using Seq2 = Seq1::ExceptSequence<thes::AutoSequence<2>>;
    static_assert(std::same_as<Seq2, thes::ValueSequence<int>>);
  }
  {
    using Seq1 = thes::AutoSequence<1, 3, 5>;
    static_assert(Seq1::all_different);
  }
  {
    using Seq1 = thes::AutoSequence<1, 3, 1>;
    static_assert(!Seq1::all_different);
  }
}
