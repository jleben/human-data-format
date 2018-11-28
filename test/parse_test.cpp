#include "parse_test.h"
#include "../event_printing.h"

#include <fstream>
#include <sstream>

using namespace std;

namespace human_data {

bool test_parse_file(const string & input_file, const vector<Parser2::Event> & expected_events)
{
    ifstream input(input_file);

    Testing::assert_critical("Input file not open.", input.is_open());

    return test_parse(input, expected_events);
}

bool test_parse(const string & text, const vector<Parser2::Event> & expected_events)
{
    istringstream input(text);

    return test_parse(input, expected_events);
}

bool test_parse(istream & input, const vector<Parser2::Event> & expected_events)
{
    Parse_Test test(expected_events);

    Parser2 parser(input, test);
    parser.parse();

    test.evaluate();

    return test.success();
}

void Parse_Test::event(const Parser2::Event &event)
{
    d_actual_events.push_back(event);
}

void Parse_Test::evaluate()
{
    bool ok = d_actual_events.size() == d_expected_events.size();

    if (ok)
    {
        for (int i = 0; i < d_actual_events.size(); ++i)
        {
            if (d_actual_events[i] != d_expected_events[i])
            {
                ok = false;
                break;
            }
        }
    }

    assert(ok)
            << "Received events:" << endl
            << Event_List_Printer(d_actual_events)
            << "Expected events:" << endl
            << Event_List_Printer(d_expected_events);
}



}
