#include "parser2.h"
#include "buffered_input_stream.h"
#include <vector>
#include <regex>

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

    skip_space_across_lines();

    if (!eos())
        throw Syntax_Error("Unexpected content.", location());
}

void Parser2::node()
{
    cerr << location() << " Node." << endl;

    skip_space_across_lines();

    int start_pos = d_column;

    if(try_string("- ") || try_string("-\n"))
    {
        block_list(start_pos);
        return;
    }

    string scalar_value;
    if (!try_plain_scalar(scalar_value))
        throw Syntax_Error("Expected scalar.", location());

    skip_space();

    if (try_string(": ") || try_string("-\n"))
    {
        block_map(start_pos, scalar_value);
        return;
    }

    d_output.event(Event(Event::Scalar, scalar_value));
}

bool Parser2::try_plain_scalar(string & value)
{
    Buffered_Input_Stream buffered_input(d_input);

    // word: \w+
    // dec int:   [-+]? [0-9]+
    // octal int: 0o [0-7]+
    // hex int:   0x [0-9a-fA-F]+
    // float:  [-+]? ( \. [0-9]+ | [0-9]+ ( \. [0-9]* )? ) ( [eE] [-+]? [0-9]+ )?

    std::match_results<Buffered_Input_Stream::Iterator> results;

    std::regex pattern(
                "^("
                "\\w+"
                "|([-+]?[0-9]+)"
                "|(0o[0-7]+)"
                "|(0x[0-9a-fA-F]+)"
                "|([-+]?(\\.[0-9]+|[0-9]+(\\.[0-9]*)?)([eE][-+]?[0-9]+)?)"
                ")");

    bool ok = regex_search(buffered_input.begin(), buffered_input.end(),
                           results, pattern);
    if (ok)
    {
        value = results.str();
        auto match_end_pos = buffered_input.position(results[0].second);
        d_column += results.length();
        d_input.seekg(match_end_pos);
        return true;
    }
    else
    {
        d_input.seekg(buffered_input.start_position());
        return false;
    }
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

            skip_space_in_flow(min_indent);

            if (!optional_flow_comma())
            {
                if (get() == ']') break;
                else
                {
                    unget();
                    throw Syntax_Error("Unexpected content. Expecting ']'.", location());
                }
            }
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

// Entered after first "- " in a block list.
void Parser2::block_list(int min_indent)
{
    cerr << location() << " Block list." << endl;

    d_output.event(Event::List_Start);

    while(true)
    {
        skip_space_across_lines();

        if (d_column <= min_indent)
            throw Syntax_Error(("Expected list element at column > ")
                               + to_string(min_indent) + ".",
                               location());

        node();

        skip_space_across_lines();

        if (d_column < min_indent)
            break;

        if (d_column != min_indent)
            throw Syntax_Error(string("Expected list bullet '-' at column ")
                               + to_string(min_indent) + ".",
                               location());

        bool found_next_bullet = try_string("- ") || try_string("-\n");
        if (!found_next_bullet)
            throw Syntax_Error("Expected list entry starting with '-'.", location());
    }

    d_output.event(Event::List_End);
}

// Entered after first "key:" in a map
void Parser2::block_map(int start_pos, string first_key)
{
    cerr << location() << " Block map anchored at " << start_pos << endl;

    d_output.event(Event::Map_Start);

    d_output.event(Event(Event::Map_Key, first_key));

    while(true)
    {
        skip_space_across_lines();

        if (d_column <= start_pos)
            throw Syntax_Error(string("Expected value for a key-value pair at column > ")
                               + to_string(start_pos) + ",",
                               location());

        node();

        skip_space_across_lines();

        if (d_column < start_pos)
            break;

        if (d_column != start_pos)
            throw Syntax_Error("Expected a key-value pair at column "
                               + to_string(start_pos) + ".",
                               location());

        string key;
        if (!try_plain_scalar(key))
            throw Syntax_Error("Mapping key is not a scalar.", location());

        bool found_key_value_separator =
                try_string(": ") || try_string(":\n");
        if (!found_key_value_separator)
            throw Syntax_Error("Expected key-value separator.", location());

        d_output.event(Event(Event::Map_Key, key));
    }

    d_output.event(Event::Map_End);
}

void Parser2::block_scalar() {}


void Parser2::skip_space_in_flow(int min_indent)
{
    skip_space_across_lines();

    if (d_column < min_indent)
    {
        throw Syntax_Error("Flow: Indentation too low.", location());
    }
}

void Parser2::skip_space_across_lines()
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

void Parser2::skip_space()
{
    char c;
    while(try_get(c))
    {
        if (c != ' ')
        {
            unget();
            break;
        }
    }
}

bool Parser2::try_string(const string & s)
{
    auto start_pos = d_input.tellg();
    auto start_loc = location();

    try {
        for(int i = 0; i < s.size(); ++i)
        {
            char c = get();
            if (c != s[i])
            {
                d_input.seekg(start_pos);
                d_column = start_loc.column;
                d_line = start_loc.line;
                return false;
            }
            if (c == '\n')
                new_line();
        }
    }
    catch (EOS_Error &)
    {
        // no op
    }

    return true;
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

void Parser2::unget(int count)
{
    while(count--)
    {
        d_input.unget();
        --d_column;
    }
}

void Parser2::unget(std::string & s, int count)
{
    while(count--)
    {
        unget();
        s.pop_back();
    }
}


}
