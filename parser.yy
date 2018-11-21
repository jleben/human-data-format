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
  flow_node
  { params.root = $1; }
| block_node INDENT_DOWN
  { params.root = $1; }
;

/*
node:
  flow_node
| block_node
;
*/

flow_node:
  scalar
;

block_node:
  block_list
;

scalar:
  LETTER
| scalar LETTER
  {
    $1->value += $2->value;
    $$ = $1;
  }
;

block_list:
  block_start block_list_elements
  { $$ = $2; }
;

block_list_elements:
  block_list_element
| block_list_elements block_list_element
  { $1->add_children($2->children); }
;

block_list_element:
  '-' space flow_node NEWLINE
  { $$ = make_node(node_type::list, { $3 }); }
| '-' space block_node INDENT_DOWN
  { $$ =  make_node(node_type::list, { $3 }); }
| '-' space_opt NEWLINE INDENT_UP block_node INDENT_DOWN
  { $$ = make_node(node_type::list, { $5 }); }
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
