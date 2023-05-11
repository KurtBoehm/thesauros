#include <concepts>
#include <tuple>
#include <utility>

#include "thesauros/macropolis.hpp"
#include "thesauros/utility.hpp"

using namespace thes::literals;

#if true
struct Test1 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test1), CONSTEXPR_CONSTRUCTOR, (SNAKE_CASE(CapitalName), int),
                   (KEEP(test), float, 2.F))

  Test1(Test1&&) noexcept = default;
  Test1(const Test1&) = delete;
};
#else
struct Test1 {
  constexpr explicit Test1(int capital_name) : CapitalName(capital_name) {}

  int CapitalName;
  float test = 2.F;
};
template<>
struct thes::TypeInfo<Test1> {
  static constexpr auto name = "Test1"_sstr;
  static constexpr auto serial_name = "test1"_sstr;
  static constexpr std::tuple members{
    MemberInfo<int, "CapitalName"_sstr, "capital_name"_sstr, &Test1::CapitalName>{},
    MemberInfo<float, "test"_sstr, "test"_sstr, &Test1::test>{}};
};
#endif

inline constexpr Test1 test1{2};

using Test1Info = thes::TypeInfo<Test1>;
static_assert(Test1Info::name == "Test1"_sstr);
static_assert(Test1Info::serial_name == "test1"_sstr);

inline constexpr auto test_1a = std::get<0>(Test1Info::members);
using Test1a = decltype(test_1a);
static_assert(std::same_as<Test1a::Type, int>);
static_assert(Test1a::name == "CapitalName"_sstr);
static_assert(Test1a::serial_name == "capital_name"_sstr);
static_assert(Test1a::pointer == &Test1::CapitalName);

inline constexpr auto test_1b = std::get<1>(Test1Info::members);
using Test1b = decltype(test_1b);
static_assert(std::same_as<Test1b::Type, float>);
static_assert(Test1b::name == "test"_sstr);
static_assert(Test1b::serial_name == "test"_sstr);
static_assert(Test1b::pointer == &Test1::test);

////////////////////////////////////////////////////////////////

#define USE_NAMESPACE true

#if USE_NAMESPACE
#if false
THES_DEFINE_ENUM(Inner, SNAKE_CASE(Test2), int, LOWERCASE(A), LOWERCASE(B));
#else
namespace inner {
enum struct Test2 { A, B };
}
THES_DEFINE_ENUM_INFO(inner, SNAKE_CASE(Test2), LOWERCASE(A), LOWERCASE(B));
#endif
#else
THES_DEFINE_ENUM_SIMPLE(SNAKE_CASE(Test2), int, SNAKE_CASE(A));
#endif

#if USE_NAMESPACE
using Type2 = inner::Test2;
#else
using Type2 = Test2;
#endif
using Test2Info = thes::EnumInfo<Type2>;

static_assert(Test2Info::name == "Te"_sstr + "st2"_sstr);
static_assert(Test2Info::serial_name == "test2"_sstr);

inline constexpr auto test_2_a = std::get<0>(Test2Info::values);
using Test2A = decltype(test_2_a);
static_assert(Test2A::name == "A"_sstr);
static_assert(Test2A::serial_name == "a"_sstr);

static_assert(thes::enum_value_info<inner::Test2::A>.name == "A"_sstr);
static_assert(thes::enum_value_info<inner::Test2::B>.name == "B"_sstr);

////////////////////////////////////////////////////////////////

template<Type2 tValue>
struct Test3 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test3, tValue), CONSTEXPR_CONSTRUCTOR)
};
using Type3 = Test3<Type2::A>;
using Test3Info = thes::TypeInfo<Type3>;
static_assert(Test3Info::name == "Test3"_sstr);
static_assert(Test3Info::serial_name == "test3_a"_sstr);

////////////////////////////////////////////////////////////////

template<typename T, Type2 tValue2>
struct Test4 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test4, (T, tValue2)), CONSTEXPR_CONSTRUCTOR)
};
using Type4 = Test4<Type2, Type2::B>;
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

template<Type2 tVal, typename TType>
struct Test7 {
  THES_DEFINE_TYPE_EX(SNAKE_CASE(Test7), CONSTEXPR_CONSTRUCTOR,
                      TEMPLATE_PARAMS((Type2)tVal, (typename)TType),
                      MEMBERS((SNAKE_CASE(a), TType), (SNAKE_CASE(b), int)),
                      STATIC_MEMBERS(("value", tVal), ("type", thes::type_tag<TType>)))
  Test7(Test7&&) noexcept = default;
  Test7(const Test7&) = delete;

#if true
  THES_DEFINE_FLATTEN_TYPE((tVal, TNewType), (TNewType)a)
#else
  constexpr auto flatten_variants() && {
    using Self = typename TypeInfo::Type;
    using MemberInfos = typename TypeInfo::Members;
    constexpr auto member_infos = TypeInfo::members;

    constexpr std::tuple variant_members{&Self::a};

    constexpr auto make_index_sequence = [](const auto& tuple) {
      return std::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(tuple)>>>{};
    };
    constexpr auto variant_index_of = [=]<std::size_t tIdx>(std::index_sequence<tIdx>) {
      constexpr auto ptr = std::get<tIdx>(member_infos).pointer;
      std::optional<std::size_t> index = std::nullopt;
      [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
        return ((::thes::Impl::member_ptrs_eq(std::get<tIdxs>(variant_members), ptr)
                   ? (index = tIdxs, true)
                   : false) ||
                ...);
      }(make_index_sequence(variant_members));
      return index;
    };
    return ::thes::fancy_flat_visit(
      [&]<typename TNewTypeTemp>(auto maker1, TNewTypeTemp&& p_a_temp) {
        return ::thes::fancy_visit_with_maker(
          maker1,
          [&]<typename TNewType>(auto maker2, TNewType&& p_a) {
            using Out = typename TypeInfo::template TemplateType<tVal, TNewType>;

            using Visited = std::tuple<TNewType>;
            Visited visited{std::forward<TNewType>(p_a)};

            auto impl = [&]<std::size_t tIdx>(std::index_sequence<tIdx>) -> decltype(auto) {
              constexpr auto variant_idx = variant_index_of(std::index_sequence<tIdx>{});
              if constexpr (variant_idx.has_value()) {
                return std::forward<std::tuple_element_t<*variant_idx, Visited>>(
                  std::get<*variant_idx>(visited));
              } else {
                return std::forward<typename std::tuple_element_t<tIdx, MemberInfos>::Type>(
                  this->*std::get<tIdx>(member_infos).pointer);
              }
            };
            return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
              return maker2(std::in_place_type<Out>, impl(std::index_sequence<tIdxs>{})...);
            }(make_index_sequence(member_infos));
          },
          ::thes::flatten_variants(std::forward<TNewTypeTemp>(p_a_temp)));
      },
      std::forward<typename ::thes::Impl::MemberInfoTypeOf<TypeInfo, &Self::a>::Type>(a));
  }
#endif
};

using Type7a = Test7<Type2::A, int>;
using Type7aInfo = thes::TypeInfo<Type7a>;
inline constexpr auto mems = Type7aInfo::members;
inline constexpr auto stat_mems = Type7aInfo::static_members;
inline constexpr auto mem0 = std::get<0>(stat_mems);
inline constexpr auto mem1 = std::get<1>(stat_mems);

static_assert(mem0.serial_name == "value"_sstr);
static_assert(mem0.value == Type2::A);
static_assert(mem1.serial_name == "type"_sstr);

static_assert(thes::serial_value(mem0.value) == "a"_sstr.view());
static_assert(thes::serial_value(mem1.value) == "i32");

using Type7b = Test7<Type2::A, std::variant<int, double>>;
using Type7c = Test7<Type2::B, std::optional<std::variant<int, Type7b>>>;

constexpr auto test7fv = thes::flatten_type(Type7b{4, 4});
static_assert(std::same_as<std::decay_t<decltype(test7fv)>,
                           std::variant<Test7<Type2::A, int>, Test7<Type2::A, double>>>);

constexpr auto test7fv2 = thes::flatten_type(std::variant<Type7b, Type7c>{Type7b{4, 4}});
static_assert(
  std::same_as<std::decay_t<decltype(test7fv2)>,
               std::variant<Test7<inner::Test2::A, int>, Test7<inner::Test2::A, double>,
                            Test7<inner::Test2::B, std::nullopt_t>, Test7<inner::Test2::B, int>,
                            Test7<inner::Test2::B, Test7<inner::Test2::A, int>>,
                            Test7<inner::Test2::B, Test7<inner::Test2::A, double>>>>);

////////////////////////////////////////////////////////////////

static_assert(thes::serial_name_of<Test1>() == "test1"_sstr);
static_assert(thes::serial_name_of<Type2>() == "test2"_sstr);
static_assert(thes::serial_name_of<Type2::A>() == "a"_sstr);

int main() {}
