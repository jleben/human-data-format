#include "../test_lib/testing.h"
#include "parse_test.h"

#include <sstream>

using namespace Testing;
using namespace std;

namespace human_data {

bool test_plain_scalar_word()
{
    Parse_Test test;
    test.evaluate_test_file("../test/cases/scalar.txt");
    return test.success();
}

bool test_undecorated_list()
{
    Parse_Test test;

    string text1 = "one\ntwo\nthree\n";
    string text2 = "one\ntwo\nthree";

    vector<Parser2::Event> events =
    {
        Parser2::Event::List_Start,
        { Parser2::Event::Scalar, "one" },
        { Parser2::Event::Scalar, "two" },
        { Parser2::Event::Scalar, "three" },
        Parser2::Event::List_End,
    };

    test.parse(text1, events);
    test.parse(text2, events);

    return test.success();
}

Test_Set parser_tests()
{
    return {
        { "plain-scalar-word", test_plain_scalar_word },
        { "undecorated-list", test_undecorated_list },
    };
}

}
