// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_MOVE_DETECTOR_HPP
#define INCLUDE_THESAUROS_UTILITY_MOVE_DETECTOR_HPP

namespace thes {
struct MoveDetector {
  MoveDetector() = default;
  MoveDetector(const MoveDetector&) = delete;
  MoveDetector(MoveDetector&& other) noexcept : unmoved_{other.unmoved_} {
    other.unmoved_ = false;
  };
  MoveDetector& operator=(const MoveDetector&) = delete;
  MoveDetector& operator=(MoveDetector&&) noexcept = delete;
  ~MoveDetector() = default;

  [[nodiscard]] bool is_moved() const {
    return !unmoved_;
  }
  [[nodiscard]] bool is_unmoved() const {
    return unmoved_;
  }

private:
  bool unmoved_{true};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_MOVE_DETECTOR_HPP
