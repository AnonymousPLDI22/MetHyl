//
// Created by pro on 2021/8/26.
//

#ifndef DPSYNTHESISNEW_SURFACE_H
#define DPSYNTHESISNEW_SURFACE_H

#include "semantics.h"

int yyparse();
int yylex();
void scanString(const char* str);
extern void* yy_result;
struct ParseInfo {
    std::map<std::string, class::Type*> param_type_map;
    int inp_num = -1;
    std::map<std::string, int> inp_id_map;
    std::map<std::string, int> env_id_map;
    class::Type* btree_internal_type = nullptr;
    void setType(const std::string& name, class::Type* type);
    void setInpVars(const std::vector<std::string>& name_list);
    void setEnvVars(const std::vector<std::string>& name_list);
    void setBTreeInternal(class::Type* type);
};
extern ParseInfo info;

enum NodeType {Type, Semantics, Program};

void* buildProgram(const std::string& type, int n_child, ...);
void* buildTask(const std::string& type, int n_child, ...);
void* buildType(const std::string& type, int n_child, ...);
void* buildExample(const std::string& type, int n_child, ...);


#endif //DPSYNTHESISNEW_SURFACE_H
