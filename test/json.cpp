#include <iostream>
#include <string_view>

#include "thesauros/io.hpp"
#include "thesauros/macropolis.hpp"

#include "tools.hpp"

namespace test = thes::test;

struct Test1 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test1), CONSTEXPR_CONSTRUCTOR, (KEEP(a), double), (KEEP(b), int))
};

struct Test2 {
  THES_DEFINE_TYPE(SNAKE_CASE(Test2), CONSTEXPR_CONSTRUCTOR, (KEEP(c), std::string),
                   (KEEP(d), Test1))
};

int main() {
  using namespace std::string_view_literals;

  // thes::numeric_string

  THES_ASSERT(test::stringeq("3.141592653589793", thes::numeric_string(std::numbers::pi).value()));
  THES_ASSERT(
    test::stringeq("3.3621031431120935063e-4932",
                   thes::numeric_string(std::numeric_limits<long double>::min()).value()));
  THES_ASSERT(test::stringeq("3.4028235e+38",
                             thes::numeric_string(std::numeric_limits<float>::max()).value()));
  THES_ASSERT(test::stringeq("-9223372036854775808",
                             thes::numeric_string(std::numeric_limits<long>::lowest()).value()));
  THES_ASSERT(
    test::stringeq("18446744073709551615",
                   thes::numeric_string(std::numeric_limits<unsigned long>::max()).value()));

  // thes::escaped_string

  THES_ASSERT(test::stringeq("abc\\\"\\r\\n", thes::escaped_string("abc\"\r\n")));
  THES_ASSERT(test::stringeq("\\u0007", thes::escaped_string("\a")));

  // thes::json_print

  THES_ASSERT(test::stringeq("3.141592653589793", thes::json_print(std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.3621031431120935063e-4932",
                             thes::json_print(std::numeric_limits<long double>::min())));
  THES_ASSERT(test::stringeq("3.4028235e+38", thes::json_print(std::numeric_limits<float>::max())));
  THES_ASSERT(
    test::stringeq("-9223372036854775808", thes::json_print(std::numeric_limits<long>::lowest())));
  THES_ASSERT(test::stringeq("18446744073709551615",
                             thes::json_print(std::numeric_limits<unsigned long>::max())));

  THES_ASSERT(test::stringeq("\"abc\\\"\\r\\n\"", thes::json_print("abc\"\r\n")));
  THES_ASSERT(test::stringeq("\"\\u0007\"", thes::json_print("\a")));

  Test1 t1{std::numbers::pi, -1};
  THES_ASSERT(
    test::stringeq("{\n  \"a\": 3.141592653589793,\n  \"b\": -1\n}", thes::json_print(t1, {0, 2})));
  Test2 t2{"Świętość\a", t1};
  THES_ASSERT(test::stringeq("{\n  \"c\": \"Świętość\\u0007\",\n  \"d\": {\n    \"a\": "
                             "3.141592653589793,\n    \"b\": -1\n  }\n}",
                             thes::json_print(t2, {0, 2})));

  {
    std::string s{};
    thes::write_json(std::back_inserter(s), t2);
    s.push_back('\n');
    thes::write_json(std::back_inserter(s), t2, thes::Indentation{2});
    THES_ASSERT(test::stringeq(
      "{ \"c\": \"Świętość\\u0007\", \"d\": { \"a\": 3.141592653589793, \"b\": -1 } }\n{\n  \"c\": "
      "\"Świętość\\u0007\",\n  \"d\": {\n    \"a\": 3.141592653589793,\n    \"b\": -1\n  }\n}",
      s));
  }
  {
    std::string s(100, '*');
    thes::write_json(s.begin(), t2);
    THES_ASSERT(test::stringeq("{ \"c\": \"Świętość\\u0007\", \"d\": { \"a\": 3.141592653589793, "
                               "\"b\": -1 } }*****************************",
                               s));
  }

  {
    std::string s = "Władca Pierścieni — Drużyna Pierścienia, Dwie Wieże, Powrót Króla";
    THES_ASSERT(
      test::stringeq("\"Władca Pierścieni — Drużyna Pierścienia, Dwie Wieże, Powrót Króla\"",
                     thes::json_print(s)));
  }
  THES_ASSERT(test::stringeq("\"θησαυρός\"", thes::json_print("θησαυρός"sv)));
}
