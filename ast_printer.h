#pragma once

#include "ast.h"

#include <iostream>
#include <string>

namespace human_data {

using std::string;

class AST_Printer
{
public:
    void print(node_ptr & node)
    {
        using namespace std;

        if (!node)
            return;

        print_child(node);
        cout << endl;
    }

    void print_child(node_ptr & node)
    {
        using namespace std;

        cout << indent() << node_type_name(node->type);

        if (!node->value.empty())
        {
            cout << " = ";
            print_value(node);
        }

        if (node->children.size())
        {
            ++d_level;
            for (auto & child : node->children)
            {
                cout << endl;
                print_child(child);
            }
            --d_level;
        }
    }

    void print_value(node_ptr & node)
    {
        using namespace std;

        if (node->value.empty())
            return;

        int max_len = 20;

        cout << "'";
        cout << node->value.substr(0, max_len);
        if (node->value.size() > max_len)
            cout << "...";
        cout << "'";
    }

    string node_type_name(node_type type)
    {
        switch(type)
        {
        case node_type::unknown:
            return "unknown";
        case node_type::scalar:
            return "scalar";
        case node_type::list:
            return "list";
        case node_type::map:
            return "map";
        case node_type::map_element:
            return "map_element";
        case node_type::tag:
            return "tag";
        default:
            return "?";
        }
    }
private:
    string indent() { return string(d_level * 2, ' '); }
    int d_level = 0;
};

}
