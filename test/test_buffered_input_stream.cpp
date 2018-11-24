#include "../buffered_input_stream.h"

#include <sstream>
#include <string>

using namespace std;

void expect(bool condition, const string & message)
{
    if (!condition)
        throw std::runtime_error(string("Failed: " + message));
}

bool test()
{
    string x = "0123456789";
    istringstream s(x);
    human_data::Buffered_Input_Stream bs(s);

    auto i = bs.begin();
    expect(*i == '0', "i == 0");

    for (int j = 1; j <= 3; ++j)
    {
        cerr << j << endl;
        ++i;
        expect(*i == x[j], string("i == ") + x[j]);
    }
    for (int j = 2; j >= 0; --j)
    {
        cerr << j << endl;
        --i;
        expect(*i == x[j], string("i == ") + x[j]);
    }

    auto end = bs.end();
    for (int j = 0; j < x.size(); ++j)
    {
        cerr << j << endl;
        ++i;
    }

    expect(i == end, "i == end");
}

int main()
{
    try
    {
        test();
    }
    catch (std::runtime_error & e)
    {
        cerr << e.what() << endl;
        return 1;
    }
}
