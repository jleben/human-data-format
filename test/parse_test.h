#include "../parser2.h"
#include "../test_lib/testing.h"

#include <iostream>
#include <vector>

namespace human_data {

using std::istream;
using std::vector;

class Parse_Test : public Testing::Test
{
public:
    bool parse_file(const string & file_name, const vector<Parser2::Event> & expected_events);
    bool parse(const string & text, const vector<Parser2::Event> & expected_events);
    bool parse(istream & input, const vector<Parser2::Event> & expected_events);
};

class Parse_Test_Helper : public Parser_Client
{
public:
    Parse_Test_Helper(Parse_Test & test, const vector<Parser2::Event> & expected_events):
        d_test(test), d_expected_events(expected_events)
    {}

    bool evaluate();

private:
    void event(const Parser2::Event &event);

    Parse_Test & d_test;
    const vector<Parser2::Event> & d_expected_events;
    vector<Parser2::Event> d_actual_events;
};

}
