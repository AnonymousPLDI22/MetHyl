//
// Created by pro on 2021/9/14.
//

#ifndef DPSYNTHESISNEW_EXECUTOR_H
#define DPSYNTHESISNEW_EXECUTOR_H

#include "task.h"
#include "preorder.h"
#include "execution_log.h"
#include "time_guard.h"
#include <map>

enum PreOrderUsageType {
    PLAN, STATE
};

class Executor {
    DataList getAllPlanForTrans(const std::pair<int, Data>& trans, const DataList& env);
    DataList _getAllPlan(const Data& state, const DataList& env);
    State* buildState(const Data& state, const DataList& env);
    State* _buildState(const Data& state, const DataList& env);
    void buildTransition(State* s, const std::pair<int, Data>& trans, const DataList& env);
    TimeGuard guard;
public:
    Task* task;
    PreOrder* plan_order;
    EqRelation* state_relation;
    Executor(Task* task, const std::map<PreOrderUsageType, PreOrder*>& config={});
    DataList getAllPlan(const Data& state, const DataList& env);
    int execute(const Data& state, const DataList& env, double timeout = 1.0);
    ExecutionLog* getLog(const Data& state, const DataList& env, double timeout = 1.0);
    std::map<PreOrderUsageType, PreOrder*> getConfig() const;
    void verifyViaSample();
};


#endif //DPSYNTHESISNEW_EXECUTOR_H
