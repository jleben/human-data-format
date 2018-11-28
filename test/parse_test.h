#include "../parser2.h"
#include "../test_lib/testing.h"

#include <iostream>
#include <vector>

namespace human_data {

using std::istream;
using std::vector;

bool test_parse_file(const string & file_name, const vector<Parser2::Event> & expected_events);
bool test_parse(const string & text, const vector<Parser2::Event> & expected_events);
bool test_parse(istream & input, const vector<Parser2::Event> & expected_events);

class Parse_Test : public Testing::Test, public Parser_Client
{
public:
    Parse_Test(const vector<Parser2::Event> & expected_events):
        d_expected_events(expected_events)
    {}

    void evaluate();

private:
    void event(const Parser2::Event &event);

    const vector<Parser2::Event> & d_expected_events;
    vector<Parser2::Event> d_actual_events;
};

}
