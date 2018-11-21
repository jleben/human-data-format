#pragma once

#include <vector>
#include <memory>
#include <string>

namespace human_data {

using std::string;
using std::vector;

enum class node_type
{
    unknown,
    scalar,
    list,
    map,
    map_element,
    tag
};

class node;

using node_ptr = std::shared_ptr<node>;

class node
{
public:
    node() {}
    node(node_type type, const std::vector<node_ptr> & children, const string & value = string()):
        type(type), children(children), value(value) {}

    void add_children(const vector<node_ptr> & list)
    {
        children.insert(children.end(), list.begin(), list.end());
    }

    node_type type = node_type::unknown;
    std::vector<node_ptr> children;
    std::string value;
};

inline
node_ptr make_node(node_type type, const std::vector<node_ptr> & children, const string & value = string())
{
    return node_ptr(new node(type, children, value));
}

}
