#include "parser2.h"
#include <vector>

using namespace std;

namespace human_data {

string strip_space(const string & s)
{
    if (s.empty())
        return s;

    int i = 0;
    while(i < s.size() && s[i] == ' ')
        ++i;
    int j = s.size() - 1;
    while(j > i && s[j] == ' ')
        --j;

    return s.substr(i, j-i);
}

bool is_char_in(const vector<char> &s, char c)
{
    auto N = s.size();
    for(int i = 0; i < N; ++i)
        if (s[i] == c)
            return true;
    return false;
}

void Parser2::parse()
{
    reset();

    node();
}

void Parser2::node()
{
    skip_space();

    int start_pos = d_column;

    char c;
    string s;
    try {
        c = get(s);
    } catch(Error &) {
        throw Error("Expected node, but got end of stream.");
    }

    try {
        if (c == '[')
        {
            flow_list(d_column);
            goto end;
        }
        else if (c == '{')
        {
            flow_map(d_column);
            goto end;
        }
        else if (c == '|')
        {
            unget();
            block_scalar();
            goto end;
        }
        else if (c == '-')
        {
            if (get(s) == ' ')
            {
                unget();
                unget();
                block_list();
                goto end;
            }
        }

        do
        {
            c = get(s);
            if (c == ':')
            {
                c = get(s);
                if (c == ' ')
                {
                    block_map(start_pos, s.substr(0, s.size()-1));
                    goto end;
                }
            }
        } while(c != '\n');
    } catch (EOS_Error &) {
        // no op
    }

end:
    skip_space();

    if (!eos())
        throw Syntax_Error("Unexpected content.", location());

    // Got scalar. Remove trailing spaces.
    Event e(Event::Scalar, strip_space(s));
    d_output.event(e);
}

void Parser2::flow_node(int min_indent)
{
    skip_space_in_flow(min_indent);

    char c;
    string s;
    switch(c = get(s))
    {
    case '[':
        flow_list(min_indent);
        return;
    case '{':
        flow_map(min_indent);
        return;
    }

    vector<char> forbidden = { '[', ']', '{', '}' };
    bool forbidden_space;

    do
    {
        if (is_char_in(forbidden, c))
        {
            unget();
            s.pop_back();
            break;
        }
        if (forbidden_space && c == ' ')
        {
            unget();
            unget();
            s.pop_back();
            s.pop_back();
            break;
        }

        forbidden_space = false;
        if (c == ',' || c== ':')
            forbidden_space = true;
    }
    while(try_get(c, s));

    if (s.size())
    {
        d_output.event(Event(Event::Scalar, s));
    }
    else
    {
        throw Syntax_Error(string("Expected flow node, but got ") + c + ".", location());
    }
}

void Parser2::flow_list(int min_indent)
{
    d_output.event(Event::List_Start);

    while(true)
    {
        try
        {
            skip_space_in_flow(min_indent);

            if (get() == ']')
                break;

            unget();

            flow_node(min_indent);

            cout << "Location after flow node: " << d_line << ":" << d_column << endl;

            skip_space_in_flow(min_indent);

            cout << "Location after flow node and space: " << d_line << ":" << d_column << endl;

            if (!optional_flow_comma())
            {

                cout << "X: " << d_line << ":" << d_column << endl;


                if (get() == ']') break;
                else
                {
                    unget();
                    throw Syntax_Error("Unexpected content. Expecting ']'.", location());
                }
            }

            cout << "C: " << d_line << ":" << d_column << endl;

        }
        catch(EOS_Error &)
        {
            throw Syntax_Error("Flow list: Unexpected end of stream.", location());
        }
    }

    d_output.event(Event::List_End);
}

void Parser2::flow_map(int min_indent)
{

}

bool Parser2::optional_flow_comma()
{
    if (get() == ',')
    {
        if (get() == ' ')
            return true;
        unget();
    }
    unget();
    return false;
}


void Parser2::block_list() {}

void Parser2::block_map(int start_pos, string first_key)
{

}

void Parser2::block_scalar() {}


void Parser2::skip_space_in_flow(int min_indent)
{
    skip_space();

    if (d_column < min_indent)
    {
        throw Syntax_Error("Flow: Indentation too low.", location());
    }
}

void Parser2::skip_space()
{
    char c;
    while(try_get(c))
    {
        if (c == '\n')
        {
            new_line();
        }
        else if (c != ' ')
        {
            unget();
            break;
        }
    }
}

bool Parser2::try_get(char & c)
{
    if (!d_input.get(c))
        return false;
    ++d_column;
    return true;
}

bool Parser2::try_get(char & c, string & s)
{
    if (try_get(c))
    {
        s += c;
        return true;
    }
    return false;
}

char Parser2::get()
{
    char c;
    if (!d_input.get(c))
        throw Error("Unexpected end of stream.");
    ++d_column;
    return c;
}

char Parser2::get(string & s)
{
    char c = get();
    s += c;
    return c;
}

}