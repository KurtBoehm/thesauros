#ifndef INCLUDE_THESAUROS_CONTAINERS_INDEX_MARKER_HPP
#define INCLUDE_THESAUROS_CONTAINERS_INDEX_MARKER_HPP

#include <cstddef>

#include "thesauros/containers/bitset/fixed.hpp"

namespace thes {
struct IndexMarker {
  using Size = std::size_t;
  using size_type = Size;

  explicit IndexMarker(Size size) : data_(size, false), size_{size} {}

  [[nodiscard]] bool get(Size index) const {
    return data_[index];
  }
  void set(Size index, bool value) {
    data_[index] = value;
  }

  [[nodiscard]] Size size() const {
    return size_;
  }

private:
  FixedBitset<sizeof(std::size_t)> data_;
  const Size size_;
};

struct OffsetIndexMarker {
  using Size = std::size_t;
  using size_type = Size;

  OffsetIndexMarker(Size start, Size end)
      : data_(end - start, false), offset_{start}, size_{end - start} {}

  [[nodiscard]] bool get(Size index) const {
    return data_[index - offset_];
  }
  void set(Size index, bool value) {
    data_[index - offset_] = value;
  }

  [[nodiscard]] Size size() const {
    return size_;
  }

private:
  FixedBitset<sizeof(std::size_t)> data_;
  const Size offset_;
  const Size size_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_INDEX_MARKER_HPP
