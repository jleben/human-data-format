#include "scanner.h"

#include <iostream>

using namespace std;

namespace human_data {

int scanner::token_type(char c)
{
    switch(c)
    {
    case ' ':
    case '-':
    case ':':
    case ',':
    case '[':
    case ']':
    case '{':
    case '}':
    case '(':
    case ')':
        return c;
    default:
        return parser::token::NORMAL_CHAR;
    }
}

void scanner::push_indent(int indent)
{
    cout << "** Scanner: Pushing indent: " << indent << endl;
    d_indent_stack.push(indent);
}

scanner::Token scanner::end_sequence()
{
    Token token;

    if(d_indent_stack.size())
    {
        d_indent_stack.pop();

        token.location.begin = token.location.end = position();
        token.type = parser::token::INDENT_DOWN;
        return token;
    }
    else
    {
        return token;
    }
}

int scanner::yylex (parser::semantic_type* yylval, parser::location_type* yylloc)
{
    auto token = yylex_real();
    *yylval = token.value;
    *yylloc = token.location;
    return token.type;
}

scanner::Token scanner::yylex_real()
{
    Token token;

    char c;

    string space;

    while(true)
    {
        switch(d_state)
        {
        case at_start:
        case at_line_start:
        {
            cerr << endl << endl << "** Scanner at line start." << endl;

            if (!d_input.get(c))
                return end_sequence();

            if (c == '\n')
            {
                // Ignore this line.
                ++d_line;
                d_column = 1;
                continue;
            }

            if (c == ' ')
            {
                ++d_column;
                continue;
            }

            // Not white space. Return it.
            d_input.unget();

            int indent = d_column;

            if (d_indent_stack.size() && indent < d_indent_stack.top())
            {
                cerr << "** Scanner: Popping indent: " << d_indent_stack.top() << endl;
                d_indent_stack.pop();

                token.location.begin = token.location.end = position();
                token.type = parser::token::INDENT_DOWN;
                return token;
            }
            else if (d_indent_stack.size() && indent > d_indent_stack.top())
            {
                d_state = in_content;
                token.location.begin = token.location.end = position();
                token.type = parser::token::INDENT_UP;
                return token;
            }
            else
            {
                d_state = in_content;
                continue;
            }
        }
        case in_content:
        {
            cerr << "** Scanner in content." << endl;

            auto token_start = position();

            if (!d_input.get(c))
                return end_sequence();

            if (c == ' ')
            {
                ++d_column;
                space += c;
                continue;
            }

            if (space.size())
            {
                d_input.unget();
                token.location.begin = token_start;
                token.location.end = position();
                token.type = parser::token::SPACE;
                token.value = make_node(node_type::unknown, {}, space);
                return token;
            }
            else if (c == '\n')
            {
                token.location.begin = token_start;
                token.location.end = position();
                token.type = parser::token::NEWLINE;
                ++d_line;
                d_column = 1;
                d_state = at_line_start;
                return token;
            }
            else
            {
                ++d_column;
                token.location.begin = token_start;
                token.location.end = position();
                token.value = make_node(node_type::scalar, {}, string(1, c));
                token.type = token_type(c);
                return token;
            }

            break;
        }
        default:
        {
            break;
        }
        }
    }

    return token;
}

}
