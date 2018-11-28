#include "../buffered_input_stream.h"
#include "../test_lib/testing.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Testing;

namespace human_data {

static
bool test_input()
{
    Test test;

    string x = "0123456789";
    istringstream s(x);
    human_data::Buffered_Input_Stream bs(s);

    auto i = bs.begin();
    test.assert(*i == '0') << "i == 0";

    for (int j = 1; j <= 3; ++j)
    {
        cerr << j << endl;
        ++i;
        test.assert(*i == x[j]) << "i == " << x[j];
    }
    for (int j = 2; j >= 0; --j)
    {
        cerr << j << endl;
        --i;
        test.assert(*i == x[j]) << "i == " << x[j];
    }

    auto end = bs.end();
    for (int j = 0; j < x.size(); ++j)
    {
        cerr << j << endl;
        ++i;
    }

    test.assert(i == end) << "i == end";

    return test.success();
}

static
bool test_repeat()
{
    Test test;

    string x = "012345";
    istringstream s(x);
    human_data::Buffered_Input_Stream bs(s);

    vector<char> data1, data2;

    for(auto i = bs.begin(); i != bs.end(); ++i)
    {
        data1.push_back(*i);
    }

    for(auto i = bs.begin(); i != bs.end(); ++i)
    {
        data2.push_back(*i);
    }

    test.assert(data1.size() == data2.size()) << "Equal size.";
    test.assert(data1 == data2) << "Equal content.";

    return test.success();
}

bool test_empty_input()
{
    Test test;

    string x = "";
    istringstream s(x);
    human_data::Buffered_Input_Stream bs(s);

    auto b = bs.begin();
    test.assert(b == bs.end());
    return test.success();
}

Test_Set buffered_input_stream_tests()
{
    return {
        { "basic", test_input },
        { "repeatability", test_repeat },
        { "empty", test_empty_input },
    };
}

}
