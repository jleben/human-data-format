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

%token LETTER
%token SPACE
%token NEWLINE
%token INDENT_UP
%token INDENT_DOWN
%token LB // line break
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
;

block_node:
  block_list
| block_map
;

scalar:
  LETTER
| scalar LETTER
  {
    $1->value += $2->value;
    $$ = $1;
  }
;


flow_list:
  flow_child[a] flow_list_separator flow_child[b]
  {
    $$ = make_node(node_type::list, { $a, $b });
  }
|
  flow_list[list] flow_list_separator flow_child
  {
    $list->add_children({ $flow_child });
    $$ = $list;
  }
;

flow_list_separator:
  ',' space | ',' NEWLINE
;

flow_child:
  scalar
| '[' space_opt flow_node space_opt ']'
  { $$ = $flow_node; }
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
  '-' space block_start flow_node NEWLINE INDENT_DOWN
  { $$ = make_node(node_type::list, { $flow_node }); }
| '-' space block_start block_node INDENT_DOWN
  { $$ =  make_node(node_type::list, { $block_node }); }
| '-' space_opt NEWLINE INDENT_UP block_start block_node INDENT_DOWN
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
  scalar ':' space block_start flow_node NEWLINE INDENT_DOWN
  {
    auto elem = make_node(node_type::map_element, { $scalar, $flow_node });
    $$ = make_node(node_type::map, { elem });
  }
| scalar ':' space block_start block_node INDENT_DOWN
  {
    auto elem = make_node(node_type::map_element, { $scalar, $block_node });
    $$ = make_node(node_type::map, { elem });
  }
| scalar ':' space_opt NEWLINE INDENT_UP block_start block_node INDENT_DOWN
  {
    auto elem = make_node(node_type::map_element, { $scalar, $block_node });
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


space_opt:
  //empty
| SPACE
;
