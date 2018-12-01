#include "parser2.h"
#include "buffered_input_stream.h"
#include <vector>
#include <regex>
#include <sstream>

using namespace std;

namespace human_data {

string Parser2::Syntax_Error::make_message(const string & m, const Location & l)
{
    ostringstream msg;
    msg << '[' << l << "] " << m;
    return msg.str();
}

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

    skip_space_across_lines();

    node();

    skip_space_across_lines();

    if (!eos())
    {
        throw Syntax_Error("Unexpected content.", location());
    }
}

void Parser2::node()
{
    cerr << location() << " Node." << endl;

    skip_space_across_lines();

    auto start_location = location();

    if(try_string("- ") || try_string("-\n"))
    {
        block_list(start_location.column);
        return;
    }

    if (try_string("> ") || try_string(">\n"))
    {
        verbatim_scalar(start_location);
        return;
    }

    if (try_string("("))
    {
        flow_collection(start_location.column);
        return;
    }

    string scalar_value;
    if (!try_plain_scalar(scalar_value))
        throw Syntax_Error("Node: Expected scalar.", location());

    skip_space();

    if (try_string(": ") || try_string(":\n"))
    {
        block_map(start_location, scalar_value);
        return;
    }

    if (try_string(", "))
    {
        d_output.event(Event::List_Start);
        d_output.event({ Event::Scalar, scalar_value });
        flow_list(start_location.column, true);
        return;
    }

    skip_space_across_lines();

    if (!eos() && d_line > start_location.line && d_column == start_location.column)
    {
        undecorated_block_list(start_location.column, scalar_value);
        return;
    }

    d_output.event(Event(Event::Scalar, scalar_value));
}

bool Parser2::try_plain_scalar(string & value)
{
    Buffered_Input_Stream buffered_input(d_input);

    std::match_results<Buffered_Input_Stream::Iterator> results;

    auto pattern_string =
            "^("
            // words:
            "[a-zA-Z_]([a-zA-Z_ ]*[a-zA-Z_])?"
            // quoted string:
            "|('([[:print:]]|\\t)*')"
            // dec int
            "|([-+]?[0-9]+)"
            // octal int
            "|(0o[0-7]+)"
            // hex int
            "|(0x[0-9a-fA-F]+)"
            // float
            "|([-+]?(\\.[0-9]+|[0-9]+(\\.[0-9]*)?)([eE][-+]?[0-9]+)?)"
            ")";

    std::regex pattern(pattern_string, std::regex::extended);

    bool ok = regex_search(buffered_input.begin(), buffered_input.end(),
                           results, pattern);
    if (ok)
    {
        value = results.str();

        auto len = results.length();
        d_column += len;

        auto match_end_pos = buffered_input.position(results[0].second);
        if (match_end_pos >= 0)
        {
            // Regex search might have extracted beyond the match, so rewind
            d_input.seekg(match_end_pos);
        }

        return true;
    }
    else
    {
        if (buffered_input.start_position() >= 0)
            d_input.seekg(buffered_input.start_position());
        return false;
    }
}

// Entered here:
// (.
void Parser2::flow_collection(int min_indent)
{
    skip_space_in_flow(min_indent);

    if (try_string("("))
    {
        d_output.event(Event::List_Start);
        flow_collection(min_indent);
        // Continue the list
        flow_list(min_indent, false);
        return;
    }

    string scalar;
    if (try_plain_scalar(scalar))
    {
        skip_space();

        if (try_string(": "))
        {
            d_output.event(Event::Map_Start);
            d_output.event({ Event::Map_Key, scalar });
            flow_map(min_indent, false);
            return;
        }
        else
        {
            d_output.event(Event::List_Start);
            d_output.event({ Event::Scalar, scalar });
            flow_list(min_indent, false);
            return;
        }
    }

    throw Syntax_Error("Flow collection: Unexpected content.", location());
}

// Entered here:
//   (_, .
//   key: .
// Requires brackets around a flow collection
void Parser2::flow_node(int min_indent)
{
    cerr << location() << " Flow node." << endl;

    if (try_string("("))
    {
        flow_collection(min_indent);
        return;
    }

    string scalar;
    if (!try_plain_scalar(scalar))
        throw Syntax_Error("Expected scalar.", location());

    cerr << location() << " Flow node is scalar: '" << scalar << "'" << endl;

    d_output.event({ Event::Scalar, scalar });
}

// Entered here:
// (node.
// If unwrapped == true:
// node,.
void Parser2::flow_list(int min_indent, bool unwrapped)
{
    cerr << location() << " Flow list." << endl;

    if (unwrapped)
    {
        skip_space_in_flow(min_indent);

        flow_node(min_indent);
    }

    while(true)
    {
        skip_space();

        if (!(try_string(", ") || try_string(",\n")))
            break;

        // FIXME: Allow list end after comma.

        skip_space_in_flow(min_indent);

        flow_node(min_indent);
    }

    if (!unwrapped)
    {
        skip_space_in_flow(min_indent);

        if (!try_string(")"))
            throw Syntax_Error("Flow list: Expected ')'.", location());
    }

    d_output.event(Event::List_End);
}

// Entered here:
// (key:.
// If 'unwrapped == true':
// key: node;.
void Parser2::flow_map(int min_indent, bool unwrapped)
{
    cerr << location() << " Flow map." << endl;

    bool first = true;

    while(true)
    {
        if (!(first && unwrapped))
        {
            skip_space_in_flow(min_indent);

            // value
            flow_node(min_indent);

            skip_space();

            if (!(try_string("; ") || try_string(";\n")))
                break;

            // FIXME: Allow end after comma.
        }

        first = false;

        skip_space_in_flow(min_indent);

        // key
        string key;
        if (!try_plain_scalar(key))
            throw Syntax_Error("Flow map: Expected plain scalar key.", location());

        d_output.event({ Event::Map_Key, key });

        skip_space();

        if (!try_string(": "))
            throw Syntax_Error("Flow map: Expected ':' after key.", location());
    }

    if(!unwrapped)
    {
        skip_space_in_flow(min_indent);

        if (!try_string(")"))
            throw Syntax_Error("Flow map: Expected ')'.", location());
    }

    d_output.event(Event::Map_End);
}

// Entered at the beginning of the second element:
//    a
//   .b
void Parser2::undecorated_block_list(int indent, string first_element)
{
    cerr << location() << " Undecorated block list." << endl;

    d_output.event(Event::List_Start);

    d_output.event(Event(Event::Scalar, first_element));

    while(true)
    {
        int line = d_line;

        string element;
        if (!try_plain_scalar(element))
            throw Syntax_Error(string("Undecorated block list: expected plain scalar."), location());

        d_output.event(Event(Event::Scalar, element));

        skip_space_across_lines();

        if (eos())
            break;

        if (not (d_line > line))
            throw Syntax_Error(string("Undecorated block list: Expected new line."),
                               location());

        if (d_column < indent)
            break;

        if (d_column != indent)
            throw Syntax_Error(string("Undecorated block list: Unexpected  indentation."),
                               location());
    }

    d_output.event(Event::List_End);
}

// Entered after first "- " in a block list.
void Parser2::block_list(int min_indent)
{
    cerr << location() << " Block list." << endl;

    d_output.event(Event::List_Start);

    while(true)
    {
        auto bullet_line = d_line;

        skip_space_across_lines();

        if (d_column <= min_indent)
            throw Syntax_Error(("Expected list element at column > ")
                               + to_string(min_indent) + ".",
                               location());

        node();

        skip_space_across_lines();

        if (d_input.eof())
            break;

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
void Parser2::block_map(const Location & start_pos, string first_key)
{
    cerr << location() << " Block map anchored at " << start_pos << endl;

    d_output.event(Event::Map_Start);

    d_output.event(Event(Event::Map_Key, first_key));

    int key_line = start_pos.line;

    while(true)
    {
        skip_space_across_lines();

        if (d_column <= start_pos.column)
            throw Syntax_Error(string("Block map: Expected value for a key-value pair at column > ")
                               + to_string(start_pos.column) + ",",
                               location());

        if (d_line > key_line)
        {
            node();
        }
        else
        {
            flow_node(start_pos.column + 1);

            // If this is the first map entry,
            // and we find a flow map separator,
            // treat this as an unparenthesised flow map.
            if (d_line == start_pos.line &&
                    (try_string("; ") || try_string(";\n")))
            {
                flow_map(start_pos.column, true);
                return;
            }
        }

        skip_space_across_lines();

        if (d_input.eof())
            break;

        if (d_column < start_pos.column)
            break;

        if (!(d_line > key_line))
            throw Syntax_Error("Block map: Expected new line.", location());

        if (d_column != start_pos.column)
            throw Syntax_Error("Block map: Expected a key-value pair at column "
                               + to_string(start_pos.column) + ".",
                               location());

        string key;
        if (!try_plain_scalar(key))
            throw Syntax_Error("Block map: Key is not a scalar.", location());

        key_line = d_line;

        bool found_key_value_separator =
                try_string(": ") || try_string(":\n");
        if (!found_key_value_separator)
            throw Syntax_Error("Block map: Expected ':'.", location());

        d_output.event(Event(Event::Map_Key, key));
    }

    d_output.event(Event::Map_End);
}

// Entered after "> " or ">\n".
// 'start' is the location of >."
void Parser2::verbatim_scalar(const Location & start)
{
    bool multi_line = d_line > start.line;

    cerr << location() << " Verbatim scalar starting at " << start
         << ", multi line = " << multi_line
         << endl;

    string content;

    if (multi_line)
    {
        bool first_line = true;

        while(true)
        {
            skip_space_until_column(start.column);

            int line_start_col = d_column;

            string line = get_line();

            if (!line.empty() && line_start_col < start.column)
                throw Syntax_Error("Verbatim scalar: Expected content or < at column "
                                   + to_string(start.column) + ".",
                                   location());

            if (line == "<")
            {
                auto loc = location();
                auto pos = d_input.tellg();

                skip_space_across_lines();

                if (d_input.eof() || d_column < start.column)
                    break;

                // Rewind and continue normally
                d_column = loc.column;
                d_line = loc.line;
                // FIXME: Error checking:
                d_input.seekg(pos);
            }

            if (!first_line)
                content += '\n';

            content += line;

            first_line = false;
        }
    }
    else
    {
        content += get_line();
    }

    if (content.empty())
        throw Syntax_Error("Verbatim scalar has no content.", location());

    d_output.event(Event(Event::Verbatim_Scalar, content));
}


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

void Parser2::skip_space_until_column(int column)
{
    char c;
    while(d_column < column && try_get(c))
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

    for(int i = 0; i < s.size(); ++i)
    {
        char c;
        if (!try_get(c) || c != s[i])
        {
            if (start_pos >= 0)
            {
                // 'seekg' clears 'eof', so we should only do it
                // if start_pos is before end of stream.
                d_input.seekg(start_pos);
            }
            d_column = start_loc.column;
            d_line = start_loc.line;
            return false;
        }
        if (c == '\n')
            new_line();
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
        throw EOS_Error();
    ++d_column;
    return c;
}

char Parser2::get(string & s)
{
    char c = get();
    s += c;
    return c;
}

// Eats line break, if found before end of stream.
// Returned string does not include line break though.
string Parser2::get_line()
{
    string line;
    std::getline(d_input, line);
    if (d_input.eof())
        d_column += line.size();
    else
        new_line();
    return line;
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
