#pragma once

#include <exception>
#include <string>
#include <iostream>

namespace human_data {

using std::istream;
using std::string;

class Parser_Client;

class Parser2
{
public:
    class Event
    {
    public:
        enum Type
        {
            Scalar,
            Verbatim_Scalar,
            List_Start,
            List_End,
            Map_Start,
            Map_End,
            Map_Key
        };

        Event(Type t, string v): type(t), value(v) {}
        Event(Type t): type(t) {}

        Type type;
        string value;
    };

    struct Location
    {
        int line;
        int column;
    };

    class Error : public std::exception
    {
    public:
        string message;
        Error(const string & message): message(message) {}
        const char * what() const noexcept { return message.c_str(); }
    };

    class Syntax_Error : public Error
    {
    public:
        Location location;
        Syntax_Error(const string & message, const Location & location):
            Error(message), location(location) {}
    };

    class EOS_Error : public Error
    {
    public:
        EOS_Error(): Error("Unexpected end of stream.") {}
    };

    Parser2(istream & input, Parser_Client & output): d_input(input), d_output(output) {}

    void parse();

private:
    enum State
    {
        Start
    };

    istream & d_input;
    Parser_Client & d_output;
    State d_state;

    int d_line = 1;
    int d_column = 1;

    void node();
    void flow_node(int min_indent);
    void flow_list(int min_indent);
    void flow_map(int min_indent);
    bool optional_flow_comma();
    void block_list();
    void block_map(int start_pos, string first_key);
    void block_scalar();

    Location location() const
    {
        return Location { d_line, d_column };
    }

    void reset()
    {
        d_state = Start;
        d_line = 1;
        d_column = 1;
    }

    void new_line()
    {
        ++d_line;
        d_column = 1;
    }

    // Get char, append it to string, and return it
    char get();
    char get(string &);
    bool try_get(char & c);
    bool try_get(char & c, string &);
    bool eos() { return d_input.eof(); }

    void unget(int count = 1);
    void unget(std::string&, int count = 1);

    void skip_space_in_flow(int min_indent);
    void skip_space();
};

class Parser_Client
{
public:
    virtual void event(const Parser2::Event & event) = 0;
};

}
