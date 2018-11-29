#include "parse_test.h"
#include "../event_printing.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

namespace human_data {

bool Parse_Test::parse(const string & text, const vector<Parser2::Event> & expected_events)
{
    istringstream input(text);

    return parse(input, expected_events);
}

bool Parse_Test::parse(istream & input, const vector<Parser2::Event> & expected_events)
{
    Event_Recorder recorder;

    try
    {
        Parser2 parser(input, recorder);
        parser.parse();
    }
    catch (Parser2::Syntax_Error & e)
    {
        assert_critical(string("Syntax error: ") + e.what(), false);
    }

    return evaluate(recorder.events(), expected_events);
}

bool Parse_Test::evaluate_test_file(const string & test_file)
{
    ifstream input(test_file);
    assert_critical("File opened.", input.is_open());

    string data;

    string line;
    bool first = true;
    while(getline(input, line))
    {
        if (line == "####")
            break;

        if (!first)
            data += '\n';

        data += line;

        first = false;
    }

    string expected_canonical_data(istreambuf_iterator<char>(input), {});

    if (expected_canonical_data.size() && expected_canonical_data.back() != '\n')
        expected_canonical_data += '\n';

    Testing::Information() << "Input:" << endl << data << "[END]" << endl;

    Event_Recorder recorder;

    try
    {
        istringstream parser_input(data);
        Parser2 parser(parser_input, recorder);
        parser.parse();
    }
    catch (Parser2::Syntax_Error & e)
    {
        assert_critical(string("Syntax error: ") + e.what(), false);
    }

    bool data_ok = recorder.canonical() == expected_canonical_data;

    assert(data_ok)
            << "Actual:" << endl
            << recorder.canonical()
            << "Expected:" << endl
            << expected_canonical_data;

    return data_ok;
}

bool Parse_Test::evaluate(const vector<Parser2::Event> & actual,
                          const vector<Parser2::Event> & expected)
{
    bool ok = actual.size() == expected.size();

    if (ok)
    {
        for (int i = 0; i < actual.size(); ++i)
        {
            if (actual[i] != expected[i])
            {
                ok = false;
                break;
            }
        }
    }

    assert(ok)
            << "Actual events:" << endl
            << Event_List_Printer(actual)
            << "Expected events:" << endl
            << Event_List_Printer(expected);

    return ok;
}

void Event_Recorder::event(const Parser2::Event &event)
{
    d_events.push_back(event);
    d_canonical_form << event;
}

}
