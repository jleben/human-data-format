#include "../parser2.h"
#include "../test_lib/testing.h"

#include <iostream>
#include <vector>
#include <sstream>

namespace human_data {

using std::istream;
using std::vector;

class Parse_Test : public Testing::Test
{
public:
    bool parse(const string & text, const vector<Parser2::Event> & expected_events);
    bool parse(istream & input, const vector<Parser2::Event> & expected_events);
    bool evaluate_test_file(const string & test_file);
private:
    bool evaluate(const vector<Parser2::Event> & actual,
                  const vector<Parser2::Event> & expected);
};

class Event_Recorder : public Parser_Client
{
public:
    const vector<Parser2::Event> & events() { return d_events; }
    string canonical() { return d_canonical_form.str(); }

private:
    void event(const Parser2::Event &event);

    vector<Parser2::Event> d_events;
    std::ostringstream d_canonical_form;
};

}
