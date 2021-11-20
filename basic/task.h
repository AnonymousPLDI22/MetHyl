//
// Created by pro on 2021/8/29.
//

#ifndef DPSYNTHESISNEW_TASK_H
#define DPSYNTHESISNEW_TASK_H

#include "program.h"

struct TaskExample {
    Data inp, oup;
    DataList env;
    TaskExample(const Data& _inp, const DataList& _env, const Data& _oup): inp(_inp), env(_env), oup(_oup) {}
};
typedef std::vector<TaskExample> TaskExampleList;

struct EnvVar {
    std::string name;
    class::Type* type;
    EnvVar(const std::string& _name, class::Type* _type): name(_name), type(_type) {}
};

class Task {
    //DataList getAllPlanForTrans(const std::pair<int, Data>& trans, const DataList& env) const;
public:
    std::vector<EnvVar> env_list;
    class::Type *state_type, *plan_type;
    std::vector<TypeList> trans_types;
    std::vector<TaskExample> sample_example_list;
    class::Program* eval, *t;
    ProgramList f_list;
    Task(class::Type* _state, const std::vector<std::pair<std::string, class::Type*>>& vars, class::Type* _plan,
         class::Type* _trans, class::Program* _t, const ProgramList& _f_list, class::Program* _eval,
         const TaskExampleList& _sample_example_list);
    void print(FILE* file = nullptr) const;
    int evaluate(const Data& plan, const DataList& env) const;
    TypeList getEnvType() const;
};


#endif //DPSYNTHESISNEW_TASK_H
