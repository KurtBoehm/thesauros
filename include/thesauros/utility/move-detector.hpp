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
