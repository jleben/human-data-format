%code requires
{
namespace human_data { class scanner; }
}

%skeleton "lalr1.cc"

%defines

%locations

%define api.namespace {human_data}
//%define api.value.type {stream::ast::semantic_value_type}
%parse-param { class human_data::scanner& scanner }

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
#define yylex scanner.yylex
}

%%

program:
  flow_node
| block_node INDENT_DOWN
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
| LETTER scalar
;

block_list:
  block_start block_list_elements
;

block_list_elements:
  block_list_element
| block_list_element block_list_elements
;

block_list_element:
  '-' space flow_node NEWLINE
| '-' space block_node INDENT_DOWN
| '-' space_opt NEWLINE INDENT_UP block_node INDENT_DOWN
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
        int col = yyla.empty() ? scanner.column() : yyla.location.begin.column;
        scanner.push_indent(col);
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
