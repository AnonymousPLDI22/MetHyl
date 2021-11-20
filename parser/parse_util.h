//
// Created by pro on 2021/8/27.
//

#ifndef DPSYNTHESISNEW_PARSE_UTIL_H
#define DPSYNTHESISNEW_PARSE_UTIL_H

#include "task.h"

namespace parse{
    Type* parseType(const std::string& s, bool is_from_file = false);
    Program* parseProgram(const std::string& s, bool is_from_file = false);
    Type* parseLimitedType(const std::string &s, bool is_from_file = false);
    Task* parseTask(const std::string &path, bool is_from_file = false);
}


#endif //DPSYNTHESISNEW_PARSE_UTIL_H
