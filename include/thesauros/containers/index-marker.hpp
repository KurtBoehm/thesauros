#ifndef INCLUDE_THESAUROS_CONTAINERS_INDEX_MARKER_HPP
#define INCLUDE_THESAUROS_CONTAINERS_INDEX_MARKER_HPP

#include <cstddef>

#include "thesauros/containers/fixed-bitset.hpp"

namespace thes {
struct IndexMarker {
  using size_type = std::size_t;

  explicit IndexMarker(size_type size) : data_(size, false), size_{size} {}

  [[nodiscard]] bool get(size_type index) const {
    return data_[index];
  }
  void set(size_type index, bool value) {
    data_[index] = value;
  }

  [[nodiscard]] size_type size() const {
    return size_;
  }

private:
  FixedBitset<8> data_;
  const size_type size_;
};

struct OffsetIndexMarker {
  using size_type = std::size_t;

  OffsetIndexMarker(size_type start, size_type end)
      : data_(end - start, false), offset_{start}, size_{end - start} {}

  [[nodiscard]] bool get(size_type index) const {
    return data_[index - offset_];
  }
  void set(size_type index, bool value) {
    data_[index - offset_] = value;
  }

  [[nodiscard]] size_type size() const {
    return size_;
  }

private:
  FixedBitset<8> data_;
  const size_type offset_;
  const size_type size_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_INDEX_MARKER_HPP
