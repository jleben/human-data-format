#pragma once

#include "ast.h"

namespace human_data {

class scanner;

class Parser_Params
{
public:
    human_data::scanner & scanner;
    node_ptr root;
};

}
