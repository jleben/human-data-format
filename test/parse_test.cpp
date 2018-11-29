#include "parse_test.h"
#include "../event_printing.h"

#include <fstream>
#include <sstream>

using namespace std;

namespace human_data {

bool Parse_Test::parse_file(const string & input_file, const vector<Parser2::Event> & expected_events)
{
    ifstream input(input_file);

    assert_critical("Input file not open.", input.is_open());

    return parse(input, expected_events);
}

bool Parse_Test::parse(const string & text, const vector<Parser2::Event> & expected_events)
{
    istringstream input(text);

    return parse(input, expected_events);
}

bool Parse_Test::parse(istream & input, const vector<Parser2::Event> & expected_events)
{
    Parse_Test_Helper helper(*this, expected_events);

    try
    {
        Parser2 parser(input, helper);
        parser.parse();
    }
    catch (Parser2::Syntax_Error & e)
    {
        assert_critical(string("Syntax error: ") + e.what(), false);
    }

    return helper.evaluate();
}

void Parse_Test_Helper::event(const Parser2::Event &event)
{
    d_actual_events.push_back(event);
}

bool Parse_Test_Helper::evaluate()
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

    d_test.assert(ok)
            << "Received events:" << endl
            << Event_List_Printer(d_actual_events)
            << "Expected events:" << endl
            << Event_List_Printer(d_expected_events);

    return ok;
}



}
