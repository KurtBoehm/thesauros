#include <filesystem>
#include <iterator>
#include <limits>
#include <numbers>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "thesauros/io.hpp"
#include "thesauros/macropolis.hpp"
#include "thesauros/test.hpp"
#include "thesauros/utility.hpp"

namespace test = thes::test;

struct Test1 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test1), CONSTEXPR_CONSTRUCTOR, (KEEP(a), double), (KEEP(b), int))
};

THES_CREATE_TYPE(SNAKE_CASE(Test2), CONSTEXPR_CONSTRUCTOR, (KEEP(c), std::string), (KEEP(d), Test1))

THES_CREATE_TYPE(SNAKE_CASE(Test3), NORMAL_CONSTRUCTOR, (KEEP(p), std::filesystem::path),
                 (KEEP(d), Test1))

THES_CREATE_TYPE(SNAKE_CASE(Test4), NORMAL_CONSTRUCTOR, (KEEP(a), int),
                 (KEEP(b), std::vector<double>))
THES_CREATE_TYPE(SNAKE_CASE(Test4b), NORMAL_CONSTRUCTOR, (KEEP(a), int),
                 (KEEP(b), (std::unordered_map<std::string, double>)))

int main() {
  using namespace std::string_view_literals;

  // thes::numeric_string

  THES_ASSERT(test::string_eq("3.141592653589793", thes::numeric_string(std::numbers::pi).value()));
  THES_ASSERT(
    test::string_eq("3.3621031431120935063e-4932",
                    thes::numeric_string(std::numeric_limits<long double>::min()).value()));
  THES_ASSERT(test::string_eq("3.4028235e+38",
                              thes::numeric_string(std::numeric_limits<float>::max()).value()));
  THES_ASSERT(test::string_eq("-9223372036854775808",
                              thes::numeric_string(std::numeric_limits<long>::lowest()).value()));
  THES_ASSERT(
    test::string_eq("18446744073709551615",
                    thes::numeric_string(std::numeric_limits<unsigned long>::max()).value()));

  // thes::escaped_string

  THES_ASSERT(test::string_eq("abc\\\"\\r\\n", thes::escaped_string("abc\"\r\n")));
  THES_ASSERT(test::string_eq("\\u0007", thes::escaped_string("\a")));

  // thes::json_print

  THES_ASSERT(test::string_eq("3.141592653589793", thes::json_print(std::numbers::pi)));
  THES_ASSERT(test::string_eq("3.3621031431120935063e-4932",
                              thes::json_print(std::numeric_limits<long double>::min())));
  THES_ASSERT(
    test::string_eq("3.4028235e+38", thes::json_print(std::numeric_limits<float>::max())));
  THES_ASSERT(
    test::string_eq("-9223372036854775808", thes::json_print(std::numeric_limits<long>::lowest())));
  THES_ASSERT(test::string_eq("18446744073709551615",
                              thes::json_print(std::numeric_limits<unsigned long>::max())));

  THES_ASSERT(test::string_eq("\"abc\\\"\\r\\n\"", thes::json_print("abc\"\r\n")));
  THES_ASSERT(test::string_eq("\"\\u0007\"", thes::json_print("\a")));

  Test1 t1{std::numbers::pi, -1};
  THES_ASSERT(test::string_eq("{\n  \"a\": 3.141592653589793,\n  \"b\": -1\n}",
                              thes::json_print(t1, {0, 2})));
  Test2 t2{"Świętość\a", t1};
  THES_ASSERT(test::string_eq("{\n  \"c\": \"Świętość\\u0007\",\n  \"d\": {\n    \"a\": "
                              "3.141592653589793,\n    \"b\": -1\n  }\n}",
                              thes::json_print(t2, {0, 2})));

  Test3 t3{"Zażółć gęślą jaźń", t1};
  THES_ASSERT(test::string_eq("{\n  \"p\": \"Zażółć gęślą jaźń\",\n  \"d\": {\n    \"a\": "
                              "3.141592653589793,\n    \"b\": -1\n  }\n}",
                              thes::json_print(t3, {0, 2})));

  {
    std::string s = "Władca Pierścieni — Drużyna Pierścienia, Dwie Wieże, Powrót Króla";
    THES_ASSERT(
      test::string_eq("\"Władca Pierścieni — Drużyna Pierścienia, Dwie Wieże, Powrót Króla\"",
                      thes::json_print(s)));
  }
  THES_ASSERT(test::string_eq("\"θησαυρός\"", thes::json_print("θησαυρός"sv)));

  THES_ASSERT(test::string_eq("true", thes::json_print(true)));

  THES_ASSERT(test::string_eq(R"({ "a": 0, "b": [1, 2] })", thes::json_print(Test4{0, {1, 2}})));
  THES_ASSERT(test::string_eq("{\n  \"a\": 0,\n  \"b\": [\n    1,\n    2\n  ]\n}",
                              thes::json_print(Test4{0, {1, 2}}, thes::Indentation{2})));

  THES_ASSERT(test::string_eq(R"({ "a": 0, "b": { "b": 3, "a": 2 } })",
                              thes::json_print(Test4b{0, {{"a", 2}, {"b", 3}}})));
  THES_ASSERT(
    test::string_eq("{\n  \"a\": 0,\n  \"b\": {\n    \"b\": 3,\n    \"a\": 2\n  }\n}",
                    thes::json_print(Test4b{0, {{"a", 2}, {"b", 3}}}, thes::Indentation{2})));

  // thes::write_json

  {
    std::string s{};
    thes::write_json(std::back_inserter(s), t2);
    s.push_back('\n');
    thes::write_json(std::back_inserter(s), t2, thes::Indentation{2});
    THES_ASSERT(test::string_eq(
      "{ \"c\": \"Świętość\\u0007\", \"d\": { \"a\": 3.141592653589793, \"b\": -1 } }\n{\n  \"c\": "
      "\"Świętość\\u0007\",\n  \"d\": {\n    \"a\": 3.141592653589793,\n    \"b\": -1\n  }\n}",
      s));
  }
  {
    std::string s(80, '*');
    thes::write_json(s.begin(), t2);
    THES_ASSERT(test::string_eq(
      "{ \"c\": \"Świętość\\u0007\", \"d\": { \"a\": 3.141592653589793, \"b\": -1 } }*********",
      s));
  }
  {
    std::string s(80, '*');
    thes::write_json(s.begin(), t3);
    THES_ASSERT(test::string_eq(
      "{ \"p\": \"Zażółć gęślą jaźń\", \"d\": { \"a\": 3.141592653589793, \"b\": -1 } }*", s));
  }
}
