%code requires
{
#include "ast.h"
#include "parser_params.h"
}

%skeleton "lalr1.cc"

%defines

%locations

%define api.namespace {human_data}
%define api.value.type {human_data::node_ptr}
%parse-param { Parser_Params & params }

%define parse.error verbose

//%define lr.type ielr

%token NORMAL_CHAR
%token SPACE
%token NEWLINE
%token INDENT_UP
%token INDENT_DOWN
%token LB // line break
%token QUOTED_SCALAR
%token END 0

%start program


%code
{
#include "scanner.h"
#define yylex params.scanner.yylex
}

%%

program:
  block_start node INDENT_DOWN
  { params.root = $2; }
;

node:
  flow_node
| block_node
;

flow_node:
  scalar
| flow_list
| flow_map
;

top_flow_node:
  flow_node
| multiple_flow_list_elems
;

block_node:
  block_list
| block_map
;

scalar:
  plain_scalar
| QUOTED_SCALAR
;

plain_scalar:
  NORMAL_CHAR[char]
  {
    $$ = make_node(node_type::scalar, {}, $char->value );
  }
| plain_scalar[scalar] NORMAL_CHAR[char]
  {
    $scalar->value += $char->value;
    $$ = $scalar;
  }
;
| plain_scalar[scalar] SPACE[space] NORMAL_CHAR[char]
  {
    $scalar->value += $space->value;
    $scalar->value += $char->value;
    $$ = $scalar;
  }
;


flow_list:
  '[' flow_list_elems ']'
  { $$ = $flow_list_elems; }
;

flow_list_elems:
  flow_node
  {
    $$ = make_node(node_type::list, { $flow_node });
  }
|
  multiple_flow_list_elems
;

multiple_flow_list_elems:
  flow_list_elems[list] flow_separator flow_node
  {
    $list->add_children({ $flow_node });
    $$ = $list;
  }
;

flow_map:
  '{' flow_map_elems '}'
  { $$ = $flow_map_elems; }
;

flow_map_elems:
  flow_key_value
  {
    $$ = make_node(node_type::map, { $flow_key_value });
  }
|
  flow_map_elems[map] flow_separator flow_key_value
  {
    $map->add_children({ $flow_key_value });
    $$ = $map;
  }
;

flow_key_value:
  scalar[key] ':' space flow_node[value]
  { $$ = make_node(node_type::map_element, { $key, $value }); }
;

flow_separator:
  ',' space | ',' NEWLINE INDENT_UP
;



block_list:
  block_list_elements
;

block_list_elements:
  block_list_element
| block_list_elements block_list_element
  { $1->add_children($2->children); }
;

block_list_element:
  '-' space block_start top_flow_node[child] NEWLINE INDENT_DOWN
  { $$ = make_node(node_type::list, { $child }); }
| '-' space block_start block_node INDENT_DOWN
  { $$ =  make_node(node_type::list, { $block_node }); }
| '-' opt_space NEWLINE INDENT_UP block_start block_node INDENT_DOWN
  { $$ = make_node(node_type::list, { $block_node }); }
;


block_map:
  block_map_elements
;

block_map_elements:
  block_map_element
| block_map_elements block_map_element
  { $1->add_children($2->children); }
;

block_map_element:
  scalar[key] ':' space top_flow_node[value] NEWLINE
  {
    auto elem = make_node(node_type::map_element, { $key, $value });
    $$ = make_node(node_type::map, { elem });
  }
| scalar[key] ':' opt_space NEWLINE INDENT_UP block_start block_node[value] INDENT_DOWN
  {
    auto elem = make_node(node_type::map_element, { $key, $value });
    $$ = make_node(node_type::map, { elem });
  }
;

/*
block_list_content:
  '-' space node
| '-' space node NEWLINE block_list_content
;
*/

block_start: store_indent;
//block_end: INDENT_DOWN;

store_indent:
    // empty
    // Set current column as indent level
    {
        int col = yyla.empty() ? params.scanner.column() : yyla.location.begin.column;
        params.scanner.push_indent(col);
    }
;

/*
indent_opt:
  //empty
| INDENT
;
*/

space:
  SPACE
;


opt_space:
  //empty
| SPACE
;
