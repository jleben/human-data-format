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

store_indent:
    // empty
    // Set current column as indent level
    {
        if (!yyla.empty())
        {
            scanner.push_indent(yyla.location.begin.column);
        }
    }
;

program:
  node
;

node:
  scalar
| block_list
;

scalar:
  LETTER
| LETTER scalar
;

block_list:
  block_start block_list_content block_end
;

block_list_content:
  '-' space node
| '-' space node NEWLINE block_list_content
;

block_start: store_indent;
block_end: INDENT_DOWN;

/*
indent_opt:
  //empty
| INDENT
;
*/

space:
  SPACE
;

/*
space_opt:
  //empty
| SPACE
;
*/
