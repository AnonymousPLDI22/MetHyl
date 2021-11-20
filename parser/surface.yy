%{
#include <stdio.h>
#include <string>
#include <map>
#include "surface.h"

extern int yylex (void);
void* yy_result;
extern void* buildType(const std::string& type, int n_child, ...);
extern void* buildProgram(const std::string& type, int n_child, ...);
extern void* buildTask(const std::string& type, int n_child, ...);
extern void* buildExample(const std::string &type, int n_child, ...);
void yyerror(const char *s, ...);
%}

%union {
    int i_val;
    char* name;
    void* res;
};
%token <i_val> INTEGER
%token <name> TYPE
%token <name> NAME

%token TYINT TYBOOL TX TYVOID TYLIST STYPE SSEM SLTYPE SPROG IF FOREACH IN DOTS LET EMPTY SSTMT PROD SINP SSTATE SENV STRANS ST SF SPLAN SEXAMPLE SEVAL ARROW TBTREE
%nonassoc IFX
%nonassoc ELSE

%left OR
%left AND
%left LE EQ '>' DOTS
%left '+' '-'
%left '*'
%nonassoc UMINUS

%type <res> main htype type hltype ltype stmt stmt_list expr param_list arg_list prog let_aux benchmark env_list
%type <res> prog_list val val_list example_list example aux_env aux_inp foreach_aux non_empty_val_list

%%

main:
      STYPE type {yy_result = $$ = $2;}
    | SLTYPE ltype {yy_result = $$ = $2;}
    | SSTMT stmt {yy_result = $$ = $2;}
    | SPROG prog {yy_result = $$ = $2;}
    | benchmark {yy_result = $$ = $1;}
    ;
type:
      htype {$$ = buildType("Extract", 1, $1);}
    ;
ltype:
      hltype {$$ = buildType("Extract", 1, $1);}
    ;
htype:
      TYINT {$$ = buildType("Int", 0);}
    | TX {$$ = buildType("X", 0);}
    | TYBOOL {$$ = buildType("Bool", 0);}
    | TYVOID {$$ = buildType("Void", 0);}
    | TYLIST '(' htype ')' {$$ = buildType("List", 1, $3);}
    | TBTREE '(' htype ',' htype ')' {$$ = buildType("BTree", 2, $3, $5);}
    | '(' htype ')' {$$ = buildType("Bracket", 1, $2);}
    | htype '*' htype {$$ = buildType("Prod", 2, $1, $3);}
    | htype '+' htype {$$ = buildType("Sum", 2, $1, $3);}
    ;
hltype:
      TYINT '[' INTEGER ',' INTEGER ']' {$$ = buildType("LInt", 2, $3, $5);}
    | TYBOOL {$$ = buildType("Bool", 0);}
    | TYVOID {$$ = buildType("Void", 0);}
    | TYLIST '[' INTEGER ']' '(' hltype ')' {$$ = buildType("LList", 2, $3, $6);}
    | TBTREE '[' INTEGER ']' '(' hltype ',' hltype ')' {$$ = buildType("LBTree", 3, $3, $6, $8);}
    | '(' hltype ')' {$$ = buildType("Bracket", 1, $2);}
    | hltype '*' hltype {$$ = buildType("Prod", 2, $1, $3);}
    | hltype '+' hltype {$$ = buildType("Sum", 2, $1, $3);}
    ;
prog:
      NAME '(' aux_inp ')' stmt {$$ = $5;}
    ;
aux_inp:
      arg_list {$$ = buildProgram("SetInp", 1, $1);}
    ;
stmt:
      ';' {$$ = buildProgram("Skip", 0);}
    | expr ';' {$$ = $1;}
    | '{' stmt_list '}' {$$ = $2;}
    | IF '(' expr ')' stmt ELSE stmt {$$ = buildProgram("If", 3, $3, $5, $7);}
    | IF '(' expr ')' stmt %prec IFX {$$ = buildProgram("If", 2, $3, $5);}
    | FOREACH foreach_aux ',' stmt {$$ = buildProgram("ForEach", 2, $2, $4);}
    | LET let_aux IN stmt {$$ = buildProgram("Let", 2, $2, $4);}
    ;
foreach_aux:
      NAME IN expr {$$ = buildProgram("ForEachAux", 2, $1, $3);}
let_aux:
      NAME '=' expr {$$ = buildProgram("LetAux", 2, $1, $3);}
    ;
stmt_list:
      stmt {$$ = buildProgram("Semicolon", 1, $1);}
    | stmt_list stmt {$$ = buildProgram("Semicolon", 2, $1, $2);}
    ;
expr:
      NAME {$$ = buildProgram("Var", 1, $1);}
    | INTEGER {$$ = buildProgram("Int", 1, $1);}
    | expr DOTS expr {$$ = buildProgram("Callsep", 3, "..", $1, $3);}
    | EMPTY {$$ = buildProgram("Empty", 0);}
    | PROD param_list ')' {$$ = buildProgram("Prod", 1, $2);}
    | NAME '(' param_list ')' {$$ = buildProgram("Call", 2, $1, $3);}
    | '(' expr ')' {$$ = $2;}
    | '-' expr %prec UMINUS {$$ = buildProgram("Callsep", 2, "neg", $2);}
    | expr '-' expr {$$ = buildProgram("Callsep", 3, "-", $1, $3);}
    | expr '+' expr {$$ = buildProgram("Callsep", 3, "+", $1, $3);}
    | expr '*' expr {$$ = buildProgram("Callsep", 3, "*", $1, $3);}
    | expr LE expr {$$ = buildProgram("Callsep", 3, "<=", $1, $3);}
    | expr EQ expr {$$ = buildProgram("Callsep", 3, "==", $1, $3);}
    | expr AND expr {$$ = buildProgram("Callsep", 3, "&&", $1, $3);}
    | expr OR expr {$$ = buildProgram("Callsep", 3, "||", $1, $3);}
    | expr '>' expr {$$ = buildProgram("Callsep", 3, ">", $1, $3);}
    | expr '<' expr {$$ = buildProgram("Callsep", 3, "<", $1, $3);}
    | NAME '.' INTEGER {$$ = buildProgram("Access", 2, $1, $3);}
    | '\\' '(' arg_list ')' '.' '(' expr ')' {$$ = buildProgram("Lambda", 2, $3, $7);}
    ;
param_list:
      expr {$$ = buildProgram("ParamList", 1, $1);}
    | param_list ',' expr {$$ = buildProgram("ParamList", 2, $1, $3);}
    ;
arg_list:
      NAME ':' type {$$ = buildProgram("ArgList", 2, $1, $3);}
    | arg_list ',' NAME ':' type {$$ = buildProgram("ArgList", 3, $1, $3, $5);}
    ;
benchmark:
      SSTATE ltype SENV aux_env SPLAN type STRANS type
      ST prog SF prog_list SEVAL prog SEXAMPLE example_list
      {$$ = buildTask("Task", 8, $2, $4, $6, $8, $10, $12, $14, $16);}
    ;
aux_env:
      env_list {$$ = buildTask("SetEnv", 1, $1);}
    ;
env_list:
      %empty {$$ = buildTask("Env", 0);}
    | env_list NAME ':' ltype {$$ = buildTask("Env", 3, $1, $2, $4);}
    ;
prog_list:
      prog {$$ = buildTask("Prog", 1, $1);}
    | prog_list prog {$$ = buildTask("Prog", 2, $1, $2);}
    ;
val:
      INTEGER {$$ = buildExample("Int", 1, $1);}
    | EMPTY {$$ = buildExample("Empty", 0);}
    | '(' non_empty_val_list ')' {$$ = buildExample("Prod", 1, $2);}
    | '[' non_empty_val_list ']' {$$ = buildExample("List", 1, $2);}
    | '[' ']' '@' type {$$ = buildExample("EmptyList", 1, $4);}
    | '{' val ',' val ',' val '}' {$$ = buildExample("BTree", 3, $2, $4, $6);}
    | '{' val '}' '@' type {$$ = buildExample("BTree", 2, $5, $2);}
    ;
non_empty_val_list:
      val_list val {$$ = buildExample("ValList", 2, $1, $2);}
    | val_list ',' val {$$ = buildExample("ValList", 2, $1, $3);}
    ;
val_list:
      %empty {$$ = buildExample("ValList", 0);}
    | val_list val {$$ = buildExample("ValList", 2, $1, $2);}
    | val_list ',' val {$$ = buildExample("ValList", 2, $1, $3);}
    ;
example:
      val val_list ARROW val {$$ = buildExample("Example", 3, $1, $2, $4);}
    ;
example_list:
      %empty {$$ = buildExample("ExampleList", 0);}
    | example_list example {$$ = buildExample("ExampleList", 2, $1, $2);}
%%

void yyerror(const char *s, ...)
{
    fprintf(stderr, "%s\n", s);
}
