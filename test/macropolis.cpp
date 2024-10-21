#include <array>
#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>

#include "thesauros/format.hpp"
#include "thesauros/macropolis.hpp"
#include "thesauros/utility.hpp"

using namespace thes::literals;

#if false
struct Test1 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test1), CONSTEXPR_CONSTRUCTOR, (SNAKE_CASE(CapitalName), int),
                   (KEEP(test), float, 2.F))

  Test1(Test1&&) noexcept = default;
  Test1(const Test1&) = delete;
};
#else
THES_CREATE_TYPE_EX(SNAKE_CASE(Test1), CONSTEXPR_CONSTRUCTOR,
                    MEMBERS((SNAKE_CASE(CapitalName), int), (KEEP(test), float, 2.F)),
                    BODY(Test1(Test1&&) noexcept = default; Test1(const Test1&) = delete;))
#endif

inline constexpr Test1 test1{2};

using Test1Info = thes::TypeInfo<Test1>;
static_assert(Test1Info::name == "Test1"_sstr);
static_assert(Test1Info::serial_name == "test1"_sstr);

inline constexpr auto test_1a = thes::star::get_at<0>(Test1Info::members);
using Test1a = decltype(test_1a);
static_assert(std::same_as<Test1a::Type, int>);
static_assert(Test1a::name == "CapitalName"_sstr);
static_assert(Test1a::serial_name == "capital_name"_sstr);
static_assert(Test1a::pointer == &Test1::CapitalName);

inline constexpr auto test_1b = thes::star::get_at<1>(Test1Info::members);
using Test1b = decltype(test_1b);
static_assert(std::same_as<Test1b::Type, float>);
static_assert(Test1b::name == "test"_sstr);
static_assert(Test1b::serial_name == "test"_sstr);
static_assert(Test1b::pointer == &Test1::test);

////////////////////////////////////////////////////////////////

namespace inner {
THES_DEFINE_ENUM(SNAKE_CASE(Test2), int, LOWERCASE(A), LOWERCASE(B));
}

using Test2Info = thes::EnumInfo<inner::Test2>;

static_assert(Test2Info::name == "Te"_sstr + "st2"_sstr);
static_assert(Test2Info::serial_name == "test2"_sstr);

inline constexpr auto test_2_a = thes::star::get_at<0>(Test2Info::values);
using Test2A = decltype(test_2_a);
static_assert(Test2A::name == "A"_sstr);
static_assert(Test2A::serial_name == "a"_sstr);

static_assert(thes::enum_value_info<inner::Test2::A>.name == "A"_sstr);
static_assert(thes::enum_value_info<inner::Test2::B>.name == "B"_sstr);

////////////////////////////////////////////////////////////////

template<inner::Test2 tValue>
struct Test3 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test3, tValue), CONSTEXPR_CONSTRUCTOR)
};
using Type3 = Test3<inner::Test2::A>;
using Test3Info = thes::TypeInfo<Type3>;
static_assert(Test3Info::name == "Test3"_sstr);
static_assert(Test3Info::serial_name == "test3_a"_sstr);

////////////////////////////////////////////////////////////////

template<typename T, inner::Test2 tValue2>
struct Test4 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test4, (T, tValue2)), CONSTEXPR_CONSTRUCTOR)
};
using Type4 = Test4<inner::Test2, inner::Test2::B>;
using Test4Info = thes::TypeInfo<Type4>;
static_assert(Test4Info::name == "Test4"_sstr);
static_assert(Test4Info::serial_name == "test4_test2_b"_sstr);

////////////////////////////////////////////////////////////////

struct Test5 {
  THES_DEFINE_TYPE(NAMED(Test5, "test5"), CONSTEXPR_CONSTRUCTOR)
};
using Test5Info = thes::TypeInfo<Test5>;
static_assert(Test5Info::name == "Test5"_sstr);
static_assert(Test5Info::serial_name == "test5"_sstr);

////////////////////////////////////////////////////////////////

struct Test6 {
  THES_DEFINE_TYPE(NAMED(Test6, "ttt"), CONSTEXPR_CONSTRUCTOR)
};
using Test6Info = thes::TypeInfo<Test6>;
static_assert(Test6Info::name == "Test6"_sstr);
static_assert(Test6Info::serial_name == thes::StaticString<3>::filled('t'));

////////////////////////////////////////////////////////////////

template<inner::Test2 tVal, typename TType>
struct Test7 {
  THES_DEFINE_TYPE_EX(SNAKE_CASE(Test7), CONSTEXPR_CONSTRUCTOR,
                      TEMPLATE_PARAMS((inner::Test2)tVal, (typename)TType),
                      MEMBERS((SNAKE_CASE(a), TType), (SNAKE_CASE(b), int)),
                      STATIC_MEMBERS((KEEP(value), tVal), (KEEP(type), thes::type_tag<TType>)))
  Test7(Test7&&) noexcept = default;
  Test7(const Test7&) = delete;

  THES_DEFINE_FLATTEN_TYPE((tVal, TNewType), (TNewType)a)
};

////////////////////////////////////////////////////////////////

THES_CREATE_TYPE_EX(SNAKE_CASE(Test8), CONSTEXPR_CONSTRUCTOR,
                    MEMBERS((SNAKE_CASE(a), int), (SNAKE_CASE(b), bool)),
                    BODY(int test() const { return a; }))
namespace n1::n2 {
THES_CREATE_TYPE_EX(SNAKE_CASE(Test9), CONSTEXPR_CONSTRUCTOR,
                    TEMPLATE_PARAMS((int)tVal, (typename)TType),
                    MEMBERS((SNAKE_CASE(a), TType), (KEEP(b), char), (SNAKE_CASE(c), int)),
                    STATIC_MEMBERS((KEEP(value), tVal), (KEEP(type), thes::type_tag<TType>)))
}

using Type7a = Test7<inner::Test2::A, int>;
using Type7aInfo = thes::TypeInfo<Type7a>;
inline constexpr auto mems = Type7aInfo::members;
inline constexpr auto stat_mems = Type7aInfo::static_members;
inline constexpr auto mem0 = thes::star::get_at<0>(stat_mems);
inline constexpr auto mem1 = thes::star::get_at<1>(stat_mems);
inline constexpr auto mem1val = thes::star::get_at<1>(stat_mems).value;
using Mem0 = decltype(mem0);
using Mem1 = decltype(mem1);

static_assert(Mem0::serial_name == "value"_sstr);
static_assert(Mem0::value == inner::Test2::A);
static_assert(Mem1::serial_name == "type"_sstr);

static_assert(thes::serial_value(Mem0::value) == "a"_sstr.view());
static_assert(thes::serial_value(Mem1::value) == "i32");

using Type7b = Test7<inner::Test2::A, std::variant<int, double>>;
using Type7c = Test7<inner::Test2::B, std::optional<std::variant<int, Type7b>>>;

constexpr auto test7fv = thes::flatten_type(Type7b{4, 4});
static_assert(
  std::same_as<std::decay_t<decltype(test7fv)>,
               std::variant<Test7<inner::Test2::A, int>, Test7<inner::Test2::A, double>>>);

constexpr auto test7fv2 = thes::flatten_type(std::variant<Type7b, Type7c>{Type7b{4, 4}});
static_assert(
  std::same_as<std::decay_t<decltype(test7fv2)>,
               std::variant<Test7<inner::Test2::A, int>, Test7<inner::Test2::A, double>,
                            Test7<inner::Test2::B, std::nullopt_t>, Test7<inner::Test2::B, int>,
                            Test7<inner::Test2::B, Test7<inner::Test2::A, int>>,
                            Test7<inner::Test2::B, Test7<inner::Test2::A, double>>>>);

////////////////////////////////////////////////////////////////

static_assert(thes::serial_name_of<Test1>() == "test1"_sstr);
static_assert(thes::serial_name_of<inner::Test2>() == "test2"_sstr);
static_assert(thes::serial_name_of<inner::Test2::A>() == "a"_sstr);

////////////////////////////////////////////////////////////////

using Type9 = n1::n2::Test9<11, double>;
static_assert((thes::memory_layout_info<Type9> |
               thes::star::transform([](auto info) { return info.offset; }) |
               thes::star::to_array) == std::array{0_uz, 8_uz, 12_uz});

int main() {
  thes::memory_layout_info<Type9> |
    thes::star::for_each([](auto info) { fmt::print("{}: {}\n", info.name, info.offset); });
}
