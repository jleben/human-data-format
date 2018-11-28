#include "../test_lib/testing.h"
#include "parse_test.h"

#include <sstream>

using namespace Testing;
using namespace std;

namespace human_data {

bool test_plain_scalar_word()
{
    string text = "oedipus";
    vector<Parser2::Event> events =
    {
        { Parser2::Event::Scalar, "oedipus" }
    };

    return test_parse(text, events);
}

bool test_undecorated_list()
{
    string text = "one\ntwo\nthree\n";
    vector<Parser2::Event> events =
    {
        Parser2::Event::List_Start,
        { Parser2::Event::Scalar, "one" },
        { Parser2::Event::Scalar, "two" },
        { Parser2::Event::Scalar, "three" },
        Parser2::Event::List_End,
    };

    return test_parse(text, events);
}

Test_Set parser_tests()
{
    return {
        { "plain-scalar-word", test_plain_scalar_word },
        { "undecorated-list", test_undecorated_list },
    };
}

}
