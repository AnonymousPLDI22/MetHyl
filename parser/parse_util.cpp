//
// Created by pro on 2021/8/27.
//

#include "parse_util.h"
#include "surface.h"
#include <iostream>
#include <fstream>
#include <cstdio>

namespace {
    std::string loadStringFromFile(const std::string& path) {
        std::fstream f;
        f.open(path.c_str(), std::ios::in);
        static char ch[1000];
        std::string res;
        while (!f.eof()) {
            f.getline(ch, 900);
            res += " " + std::string(ch);
        }
        return res;
    }
}

class::Type * parse::parseType(const std::string &s, bool is_from_file) {
    auto text = is_from_file ? loadStringFromFile(s) : s;
    yy_result = nullptr;
    scanString(("#Type " + text).c_str());
    yyparse();
    return static_cast<class::Type*>(yy_result);
}

class::Type * parse::parseLimitedType(const std::string &s, bool is_from_file) {
    auto text = is_from_file ? loadStringFromFile(s) : s;
    yy_result = nullptr;
    scanString(("#LType " + text).c_str());
    yyparse();
    return static_cast<class::Type*>(yy_result);
}

class::Program* parse::parseProgram(const std::string &s, bool is_from_file) {
    auto text = is_from_file ? loadStringFromFile(s) : s;
    yy_result = nullptr;
    // std::cout << text << std::endl;
    scanString(("#Prog " + text).c_str());
#ifdef TESTLEX
    yylex(); exit(0);
#endif
    yyparse();
    return static_cast<class::Program*>(yy_result);
}

Task * parse::parseTask(const std::string &s, bool is_from_file) {
    auto text = is_from_file ? loadStringFromFile(s) : s;
    yy_result = nullptr;
    scanString(text.c_str());
#ifdef TESTLEX
    yylex(); exit(0);
#endif
    yyparse();
    return static_cast<Task*>(yy_result);
}