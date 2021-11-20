%{
#include <string>
#include "parser.h"
#include <iostream>
void cpyName(char*& dst, const char* src);
#ifdef TESTLEX
    #define ReturnToken(c) std::cout << #c << " ";
    #define ReturnValue(c) std::cout << c << " ";
#else
    #define ReturnToken(c) return c;
    #define ReturnValue(c) return c;
#endif
%}
%%
[\n\t ]+   ;
Bool {ReturnToken(TYBOOL);}
Int {ReturnToken(TYINT);}
X {ReturnToken(TX);}
Void {ReturnToken(TYVOID);}
List {ReturnToken(TYLIST);}
BTree {ReturnToken(TBTREE);}
#Type {ReturnToken(STYPE);}
#LType {ReturnToken(SLTYPE);}
#Prog {ReturnToken(SPROG);}
#Stmt {ReturnToken(SSTMT);}
#Semantics {ReturnToken(SSEM);}
#state {ReturnToken(SSTATE);}
#inp {ReturnToken(SINP);}
#env {ReturnToken(SENV);}
#plan {ReturnToken(SPLAN);}
#trans {ReturnToken(STRANS);}
#tprog {ReturnToken(ST);}
#fprog {ReturnToken(SF);}
#eval {ReturnToken(SEVAL);}
#example {ReturnToken(SEXAMPLE);}
if {ReturnToken(IF);}
else {ReturnToken(ELSE);}
foreach {ReturnToken(FOREACH);}
in {ReturnToken(IN);}
let {ReturnToken(LET);}
"'(" {ReturnToken(PROD);}
empty {ReturnToken(EMPTY);}
[a-z][a-zA-Z\_0-9]* {
    cpyName(yylval.name, yytext);
    ReturnToken(NAME);
}
(\-?[1-9][0-9]*)|0 {
    yylval.i_val = std::atoi(yytext);
    ReturnToken(INTEGER);
}
[\[\]()-<>=\+\*{}\.,@\\:] {
    ReturnValue(yytext[0]);
}
"<=" {ReturnToken(LE);}
"==" {ReturnToken(EQ);}
"->" {ReturnToken(ARROW);}
"\.\." {ReturnToken(DOTS);}
"\|\|" {ReturnToken(OR);}
"&&" {ReturnToken(AND);}
. {
    std::cout << yytext << std::endl;
    exit(0);
};
%%
void scanString(const char* str)
{
    yy_switch_to_buffer(yy_scan_string(str));
}
void cpyName(char*& dst, const char* src) {
    int len = strlen(src) + 1;
    dst = new char[len];
    strcpy(dst, src);
}