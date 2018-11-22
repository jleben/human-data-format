#include "scanner.h"

#include <iostream>

using namespace std;

namespace human_data {

int scanner::token_type(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return parser::token::LETTER;
    else
        return c;
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

    while(true)
    {
        switch(d_state)
        {
        case at_start:
        case at_line_start:
        {
            cerr << endl << endl << "** Scanner at line start." << endl;

            auto token_start = position();

            while(d_input.get(c))
            {
                ++d_column;
                if (c == '\n')
                {
                    // Ignore this line.
                    ++d_line;
                    d_column = 1;
                    token_start = position();
                }
                else if (c != ' ')
                {
                    d_input.unget();
                    --d_column;
                    break;
                }
            }

            if (!d_input)
                return end_sequence();

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

            int space_size = 0;
            while(d_input.get(c))
            {
                ++d_column;
                if (c != ' ')
                    break;
                space_size = d_column;
            }

            if (!d_input)
                return end_sequence();

            if (space_size)
            {
                d_input.unget();
                --d_column;
                token.location.begin = token_start;
                token.location.end = position();
                token.type = parser::token::SPACE;
                return token;
            }

            if (c == '\n')
            {
                token.location.begin = token_start;
                token.location.end = position();
                token.type = parser::token::NEWLINE;
                ++d_line;
                d_column = 1;
                d_state = at_line_start;
                return token;
            }

            token.location.begin = token_start;
            token.location.end = position();
            token.value = make_node(node_type::scalar, {}, string(1, c));
            token.type = token_type(c);
            return token;
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
