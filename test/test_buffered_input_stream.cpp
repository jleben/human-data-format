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

Test_Set buffered_input_stream_tests()
{
    return {
        { "test", test_input }
    };
}

}
