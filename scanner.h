#include "parser.hpp"

#include <iostream>
#include <stack>

namespace human_data {

using std::istream;

class scanner
{
    istream & d_input;
    std::stack<int> d_indent_stack;

    enum State
    {
        at_start,
        at_line_start,
        at_content_start,
        in_content
    };

    State d_state = at_start;

    int d_line = 0;
    int d_column = 0;

    int token_type(char c);
    int end_sequence(parser::semantic_type* yylval, parser::location_type* yylloc);

public:
    human_data::position position()
    {
        return human_data::position(nullptr, d_line, d_column);
    }

    scanner(istream & in):
        d_input(in)
    {
    }

    void push_indent(int indent);

    int yylex (parser::semantic_type* yylval, parser::location_type* yylloc);
    int yylex_real (parser::semantic_type* yylval, parser::location_type* yylloc);
};

}
