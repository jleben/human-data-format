#include "parser.hpp"
#include "scanner.h"
#include "ast_printer.h"
#include "parser2.h"

#include <fstream>
#include <iostream>

using namespace std;
using namespace human_data;

int parse_with_bison(istream & input)
{
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

    return 0;
}

class My_Parser_Client : public Parser_Client
{
    void event(const Parser2::Event & event) override
    {
        switch(event.type)
        {
        case Parser2::Event::List_Start:
        {
            cout << "[" << endl;
            break;
        }
        case Parser2::Event::List_End:
        {
            cout << "]" << endl;
            break;
        }
        case Parser2::Event::Map_Start:
        {
            cout << "{" << endl;
            break;
        }
        case Parser2::Event::Map_End:
        {
            cout << "}" << endl;
            break;
        }
        case Parser2::Event::Map_Key:
        {
            cout << "? " << event.value << endl;
            break;
        }
        case Parser2::Event::Scalar:
        {
            cout << "= " << event.value << endl;
            break;
        }
        case Parser2::Event::Verbatim_Scalar:
        {
            cout << ">" << endl << event.value << endl << '<' << endl;
            break;
        }
        default:
            cout << "Unexpected event." << endl;
        }
    }
};

void parse_custom(istream & input)
{
    My_Parser_Client client;
    Parser2 parser(input, client);

    try
    {
        parser.parse();
    }
    catch (Parser2::Syntax_Error & e)
    {
        cerr << "Syntax error: "
             << e.what()
             << endl;
    }
}

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

    parse_custom(input);
}
