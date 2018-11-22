#include "scanner.h"

#include <iostream>
#include <stdexcept>

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
    case '"':
    case '\'':
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

            if (!get(c))
                return end_sequence();

            if (c == '\n')
            {
                // Ignore this line.
                new_line();
                continue;
            }

            if (c == ' ')
            {
                continue;
            }

            // Not white space. Return it.
            unget();

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

            if (!get(c))
                return end_sequence();

            if (c == ' ')
            {
                space += c;
                continue;
            }

            if (space.size())
            {
                unget();
                token.location.begin = token_start;
                token.location.end = position();
                token.type = parser::token::SPACE;
                token.value = make_node(node_type::unknown, {}, space);
                return token;
            }
            else if (c == '\'' || c == '"')
            {
                return read_quoted_scalar(c);
            }
            else if (c == '\n')
            {
                token.location.begin = token_start;
                token.location.end = position();
                token.type = parser::token::NEWLINE;
                new_line();
                d_state = at_line_start;
                return token;
            }
            else
            {
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

scanner::Token scanner::read_quoted_scalar(char quote)
{
    Token token;
    token.location.begin = position();
    // We have already eaten the opening quote, so adjust colum.
    token.location.begin.column -= 1;

    string value;
    char c;
    bool closed = false;
    while(get(c))
    {
        if (c == quote)
        {
            closed = true;
            break;
        }
        else if (c == '\n')
        {
            break;
        }
        else
        {
            value += c;
        }
    }

    if (!closed)
        throw std::runtime_error("Quoted scalar without closing quote.");

    token.location.end = position();
    token.value = make_node(node_type::scalar, {}, value);
    token.type = parser::token::QUOTED_SCALAR;

    return token;
}

void scanner::eat_space(int max_column)
{
    char c;
    while((max_column < 0 || d_column < max_column) && get(c))
    {
        if (c != ' ')
        {
            unget();
            break;
        }
    }
}

void scanner::new_line()
{
    ++d_line;
    d_column = 1;
}

bool scanner::get(char & c)
{
    d_input.get(c);
    if (d_input) ++d_column;
    return bool(d_input);
}

void scanner::unget()
{
    d_input.unget();
    --d_column;
}

}
