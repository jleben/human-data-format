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

int scanner::end_sequence(parser::semantic_type* yylval, parser::location_type* yylloc)
{
    if(d_indent_stack.size())
    {
        yylloc->begin = yylloc->end = position();
        d_indent_stack.pop();
        return parser::token::INDENT_DOWN;
    }
    else
    {
        return 0;
    }
}

int scanner::yylex (parser::semantic_type* yylval, parser::location_type* yylloc)
{
    int token = yylex_real(yylval, yylloc);
    return token;
#if 0
    if (token == 0)
    {
        cout << "END" << endl;
    }
    else if (token < 128)
    {
        cout << char(token) << endl;
    }
    else
    {
        cout << token;
    }
#endif
}

int scanner::yylex_real (parser::semantic_type* yylval, parser::location_type* yylloc)
{
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
                    d_column = 0;
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
                return end_sequence(yylval, yylloc);

            int indent = d_column;

            if (d_indent_stack.size() && indent < d_indent_stack.top())
            {
                cerr << "** Scanner: Popping indent: " << d_indent_stack.top() << endl;
                d_indent_stack.pop();

                yylloc->begin = yylloc->end = position();
                return parser::token::INDENT_DOWN;
            }
            else if (d_indent_stack.size() && indent > d_indent_stack.top())
            {
                yylloc->begin = yylloc->end = position();
                return parser::token::INDENT_UP;
            }
            else if (d_indent_stack.size())
            {
                d_state = in_content;

                yylloc->begin = yylloc->end = position();
                return parser::token::NEWLINE;
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
                return end_sequence(yylval, yylloc);

            if (space_size)
            {
                d_input.unget();
                --d_column;
                yylloc->begin = token_start;
                yylloc->end = position();
                return parser::token::SPACE;
            }

            if (c == '\n')
            {
                ++d_line;
                d_column = 0;
                d_state = at_line_start;
                continue;
            }

            yylloc->begin = token_start;
            yylloc->end = position();
            return token_type(c);
        }
        default:
        {
            return 0;
        }
        }
    }
}

}
