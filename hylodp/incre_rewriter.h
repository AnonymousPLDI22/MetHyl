//
// Created by pro on 2021/10/15.
//

#ifndef DPSYNTHESISNEW_INCRE_REWRITER_H
#define DPSYNTHESISNEW_INCRE_REWRITER_H

#include "task.h"
#include "input_sampler.h"
#include "autolifter/autolifter.h"
#include "hylodp_basic.h"

struct PlanValueInfo {
    int f_id;
    Program* pos;
    std::vector<int> trace;
    Program* rep;
    void print() const;
    void getFullResult(const ProgramList& cared_list, Task* task);
};

struct PlanModInfo {
    int f_id;
    Program* pos;
    Program* m;
    Type* F;
    ProgramList tmp_list;
    ExampleSpace* example_space;
    void print() const;
};

class PlanCollectExampleSpace: public ExampleSpace {
    Example* insertEnv(const Data& inp, const DataList& env);
public:
    InputSampler* sampler;
    Program* f;
    Type* F;
    int f_id;
    PlanCollectExampleSpace(InputSampler* _sampler, Program* _f, int _id, Type* _F);
    virtual void acquireMoreExamples();
};

class PlanRewriter {
    ProgramList normalized_f_list;
    std::vector<PlanModInfo> mod_list;
    std::vector<PlanValueInfo> value_list;
    std::vector<std::vector<Program*>> combinator_storage;
    void insertCaredValues(Program* p);
    bool extractValues(int f_id, Program* p);
    void extractCaredComponents(int f_id, Program* p);
    void insertModifier(PlanModInfo m_info);
    void extractModifierFromCollect(int f_id, Program* p);
    void extractModifier(int f_id, Program* p);
    bool isValue(Program* p, int id);
    void buildNewTask();
    void buildNewOrder();
    int getModIndex(Program* p, int id);
    Program* rewriteForValue(Type* t, Program* p, int f_id, std::vector<int>& trace);
    PlanValueInfo getValueInfo(Program* p, std::vector<int>& trace, int id);
    Program* rewriteF(Program* p, int id);
public:
    ProgramList cared_values;
    InputSampler* sampler;
    Task* task, *new_task;
    PreOrder* plan_order, *new_order;
    Type* x_type;
    TimeGuard* guad = nullptr;
    PlanRewriter(InputSampler* _sampler, PreOrder* _plan_order, int timeout = -1);
    void rewrite();
    Task* getNewTask() const {return new_task;}
    Executor* getNewExecutor() const {return new Executor(new_task, {{PLAN, new_order}});}
};


#endif //DPSYNTHESISNEW_INCRE_REWRITER_H
