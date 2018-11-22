#pragma once

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

    struct Token
    {
        int type = 0;
        parser::semantic_type value;
        parser::location_type location;
    };

    State d_state = at_start;

    int d_line = 1;
    int d_column = 1;

    int token_type(char c);
    scanner::Token end_sequence();

public:

    int column() const { return d_column; }
    int line() const { return d_line; }

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
    Token yylex_real ();
};

}
