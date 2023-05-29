#ifndef INCLUDE_THESAUROS_IO_PRINTERS_HPP
#define INCLUDE_THESAUROS_IO_PRINTERS_HPP

#include <iterator>
#include <ostream>
#include <string_view>
#include <utility>

namespace thes {
namespace impl {
template<typename TRange>
concept IsRange = requires(const TRange& r) {
  std::begin(r);
  std::end(r);
};
} // namespace impl

template<typename TRange, typename TOp>
struct RangePrinter {
  RangePrinter(const TRange& range, TOp operation, std::string_view delimiter,
               std::string_view prefix = {}, std::string_view suffix = {})
      : range_{range}, operation_{std::move(operation)}, delimiter_{delimiter}, prefix_{prefix},
        suffix_{suffix} {}

  friend std::ostream& operator<<(std::ostream& stream, const RangePrinter& printer) {
    stream << printer.prefix_;

    auto it{std::begin(printer.range_)};
    const auto end{std::end(printer.range_)};

    if (it == end) {
      stream << printer.suffix_;
      return stream;
    }

    printer.operation_(stream, *it);
    for (++it; it != end; ++it) {
      stream << printer.delimiter_;
      printer.operation_(stream, *it);
    }

    stream << printer.suffix_;
    return stream;
  }

private:
  const TRange& range_;
  TOp operation_;
  std::string_view delimiter_, prefix_, suffix_;
};

template<typename T>
struct Printer {
  explicit Printer(const T& value) : value_{value} {}

  std::ostream& print(std::ostream& stream) const {
    return stream << value_;
  }

private:
  const T& value_;
};

template<typename TRange>
requires impl::IsRange<TRange>
struct Printer<TRange> {
  std::ostream& print(std::ostream& stream) const {
    return stream << RangePrinter{
             range, []<typename T>(std::ostream& s1, const T& x) { s1 << Printer<T>{x}; }, ", ",
             "[", "]"};
  }

  const TRange& range;
};
template<typename T1, typename T2>
struct Printer<std::pair<T1, T2>> {
  std::ostream& print(std::ostream& stream) const {
    return stream << '(' << Printer<T1>{pair.first} << ", " << Printer<T2>{pair.second} << ")";
  }

  const std::pair<T1, T2>& pair;
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Printer<T>& printer) {
  return printer.print(stream);
}

template<typename T>
auto print(const T& value) {
  return Printer<T>{value};
}

template<typename TRange>
auto range_print(const TRange& range, std::string_view delimiter = ", ",
                 std::string_view prefix = "[", std::string_view suffix = "]") {
  return RangePrinter{range, [](std::ostream& stream, const auto& x) { stream << Printer{x}; },
                      delimiter, prefix, suffix};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_PRINTERS_HPP
