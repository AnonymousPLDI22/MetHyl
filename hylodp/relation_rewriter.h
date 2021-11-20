//
// Created by pro on 2021/10/20.
//

#ifndef DPSYNTHESISNEW_RELATION_REWRITER_H
#define DPSYNTHESISNEW_RELATION_REWRITER_H

#include "input_sampler.h"
#include "example_space.h"

struct RelationModInfo {
    Program* pos;
    std::vector<int> trace;
    Program* m;
    Type* F;
    ProgramList used_tmps;
    ExampleSpace* example_space;
    Program* rewriteCombinator(Program* c, Task* t, Type* x_type);
    void print() const;
};

struct RelationValueInfo {
    Program* pos;
    std::vector<int> trace;
    Program* rep;
    void updateRep(const ProgramList& cared_values, Task* task);
    void print() const;
};

class RelationRewriter {
    void extractModFromCollect(Program* cp);
    void extractModifier(Program* p);
    bool extractCaredFunctions(Program* p);
    void extractCaredComponentFromFunctions(Program* p);
    void insertModInfo(const RelationModInfo& info);
    bool isValue(Program* p);
    int getValueIndex(Program* p, const std::vector<int>& trace);
    int getModIndex(Program* p, const std::vector<int>& trace);
    Program* rewriteT(Program* p);
    Program* rewriteAllComponent(Type* t, Program* pos, std::vector<int>& trace);
    void buildNewTask();
    void buildNewRelation();
    ProgramStorage combinator_storage;
public:
    InputSampler* sampler;
    Task* task, *new_task;
    Program* let_free_t;
    KeyEqRelation* state_relation, *new_relation;
    std::vector<RelationModInfo> mod_list;
    std::vector<RelationValueInfo> value_list;
    std::vector<Program*> cared_functions;
    TimeGuard* guard = nullptr;
    RelationRewriter(InputSampler* _sampler, KeyEqRelation* _eq_relation, int timeout = -1);
    void rewrite();
    Task* getNewTask() const {return new_task;}
    Executor* getNewExecutor() const;
};


#endif //DPSYNTHESISNEW_RELATION_REWRITER_H
