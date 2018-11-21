#include "parser.hpp"
#include "scanner.h"
#include "ast_printer.h"

#include <fstream>
#include <iostream>

using namespace std;
using namespace human_data;

int main(int argc, char * argv[])
{
    if (argc < 2)
        return 1;

    string input_file_path { argv[1] };

    ifstream input(input_file_path);
    if (!input.is_open())
    {
        cerr << "Failed to open file: " << input_file_path << endl;
        return 1;
    }

    scanner s(input);

    Parser_Params params { s };
    parser p(params);

    p.set_debug_level(1);

    bool ok = p.parse() == 0;
    if (!ok)
        return 1;

    cout << endl;
    AST_Printer printer;
    printer.print(params.root);
}
