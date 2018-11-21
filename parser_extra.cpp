#include "parser.hpp"

using namespace std;

namespace human_data {

void parser::error(const location & l, const string & msg)
{
    cout << "Parse error: " << msg << endl;
}

}
