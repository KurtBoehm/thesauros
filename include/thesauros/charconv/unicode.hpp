#ifndef INCLUDE_THESAUROS_CHARCONV_UNICODE_HPP
#define INCLUDE_THESAUROS_CHARCONV_UNICODE_HPP

#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

namespace thes {
struct UnicodeDecoder {
  using CodePoint = std::uint32_t;
  enum struct State : std::uint8_t {
    ACCEPTED = 0,
    REJECTED = 12,
  };

  // Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
  // See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
  // Originally licenced under the MIT licence.
  // Modified to be more C++
  std::pair<CodePoint, State> decode(const std::uint8_t byte) noexcept {
    // Map a character to its character class.
    static const std::array<std::uint8_t, 256> char_kind = {
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0…
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1…
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2…
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 3…
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4…
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5…
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6…
      0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7…
      1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 8…
      9,  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, // 9…
      7,  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // A…
      7,  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // B…
      8,  8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C…
      2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // D…
      10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, // E…
      11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, // F…
    };

    // A transition table that maps a combination of a state of the automaton and a character class
    // to a state (multiplied by 12, i.e. the number of states)
    static const std::array<std::uint8_t, 108> trans = {
      0,  12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, // s0
      12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, // s1
      12, 0,  12, 12, 12, 12, 12, 0,  12, 0,  12, 12, // s2
      12, 24, 12, 12, 12, 12, 12, 24, 12, 24, 12, 12, // s3
      12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, // s4
      12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, // s5
      12, 12, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, // s6
      12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, // s7
      12, 36, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    };

    const std::uint8_t kind = char_kind[byte];

    codep_ = (state_ != 0) ? (byte & 0x3FU) | (codep_ << 6U) : (0xFFU >> kind) & (byte);
    state_ = trans[state_ + kind];

    return {codep_, State{state_}};
  }

  std::pair<std::uint32_t, std::string_view> decode(std::string_view str) {
    const char* end = str.end();
    for (const char* ptr = str.begin(); ptr != end; ++ptr) {
      const auto [codep, state] = decode(std::bit_cast<std::uint8_t>(*ptr));
      switch (state) {
      case State::ACCEPTED: {
        return {codep, {ptr + 1, end}};
      }
      case State::REJECTED: throw std::invalid_argument("The string is invalid!");
      default: break;
      }
    }
    throw std::invalid_argument("The string is invalid!");
  }

  [[nodiscard]] State state() const {
    return State{state_};
  }

private:
  std::uint32_t codep_{};
  std::uint8_t state_{};
};

template<typename TStr>
struct UnicodeStringView {
  using CodePoint = std::uint32_t;

  UnicodeStringView(TStr&& str) : str_(std::forward<TStr>(str)) {}

  struct Iterator {
    Iterator(char* end) : view_{end, end} {}
    Iterator(std::string_view view) : decoder_{}, view_{view} {
      if (!view_.empty()) {
        std::tie(codep_, next_view_) = decoder_.decode(view_);
      }
    }

    std::uint32_t operator*() {
      return codep_;
    }

    Iterator& operator++() {
      view_ = next_view_;
      if (!view_.empty()) {
        std::tie(codep_, next_view_) = decoder_.decode(view_);
      }
      return *this;
    }

    friend constexpr bool operator==(const Iterator& iter1, const Iterator& iter2) {
      return iter1.view_ == iter2.view_;
    }

  private:
    UnicodeDecoder decoder_{};
    std::string_view view_;
    std::string_view next_view_;
    CodePoint codep_{};
  };

  Iterator begin() const {
    return {str_};
  }
  Iterator end() const {
    return {str_.end()};
  }

private:
  TStr str_;
};

template<typename TStr>
UnicodeStringView(TStr&&) -> UnicodeStringView<TStr>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CHARCONV_UNICODE_HPP
