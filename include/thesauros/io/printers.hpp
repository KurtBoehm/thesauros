#ifndef INCLUDE_THESAUROS_IO_PRINTERS_HPP
#define INCLUDE_THESAUROS_IO_PRINTERS_HPP

#include <iterator>
#include <ostream>
#include <string_view>
#include <utility>

namespace thes {
namespace impl {
template<typename T>
concept SupportsOStream = requires(std::ostream& s, const T& v) {
  { s << v } -> std::same_as<std::ostream&>;
};
template<typename TRange>
concept IsRange = requires(const TRange& r) {
  std::begin(r);
  std::end(r);
};
} // namespace impl

template<typename TRange, typename TOp>
struct RangePrinter {
  RangePrinter(TRange&& range, TOp operation, std::string_view delimiter,
               std::string_view prefix = {}, std::string_view suffix = {})
      : range_{std::forward<TRange>(range)}, operation_{std::move(operation)},
        delimiter_{delimiter}, prefix_{prefix}, suffix_{suffix} {}

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
  TRange range_;
  TOp operation_;
  std::string_view delimiter_, prefix_, suffix_;
};
template<typename TRange, typename TOp>
RangePrinter(TRange&&, TOp, std::string_view, std::string_view, std::string_view)
  -> RangePrinter<TRange, TOp>;

template<typename T>
struct Printer;
template<impl::SupportsOStream T>
struct Printer<T> {
  explicit Printer(T&& value) : value_{std::forward<T>(value)} {}

  std::ostream& print(std::ostream& stream) const {
    return stream << value_;
  }

private:
  T value_;
};
template<typename T>
Printer(T&&) -> Printer<T>;

template<typename TRange>
requires(!impl::SupportsOStream<TRange> && impl::IsRange<TRange>)
struct Printer<TRange> {
  explicit Printer(TRange&& range) : range_{std::forward<TRange>(range)} {}

  std::ostream& print(std::ostream& stream) const {
    return stream << RangePrinter{
             range_, []<typename T>(std::ostream& s1, const T& x) { s1 << Printer<const T&>{x}; },
             ", ", "[", "]"};
  }

private:
  TRange range_;
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Printer<T>& printer) {
  return printer.print(stream);
}

template<typename T>
auto print(T&& value) {
  return Printer<T>{std::forward<T>(value)};
}

template<typename TRange>
auto range_print(TRange&& range, std::string_view delimiter = ", ", std::string_view prefix = "[",
                 std::string_view suffix = "]") {
  return RangePrinter{std::forward<TRange>(range),
                      [](std::ostream& stream, const auto& x) { stream << Printer{x}; }, delimiter,
                      prefix, suffix};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_PRINTERS_HPP
