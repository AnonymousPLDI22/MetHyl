//
// Created by pro on 2021/9/15.
//

#ifndef DPSYNTHESISNEW_EXECUTIONLOG_H
#define DPSYNTHESISNEW_EXECUTIONLOG_H

#include "data.h"
#include "program.h"
#include <set>

class Transition;
class State;

class Expansion {
public:
    Transition* trans;
    Data plan;
    DataList plan_list;
    Expansion(Transition* _trans, const Data& _plan);
    void uniquePlans();
};

class Transition {
public:
    State *s, *t;
    std::vector<Expansion*> exp_list;
    DataList plan_list;
    Transition(State* _s, State* _t);
    std::map<std::string, Expansion*> exp_map;
    Expansion* insertExpansion(const Data& plan);
};

struct TransitionValue {
    bool is_state = false;
    State* state = nullptr;
    Data v;
    explicit TransitionValue(State* _state): is_state(true), state(_state) {}
    explicit TransitionValue(const Data& _data): is_state(false), v(_data) {}
    TransitionValue() = default;
};

class TransitionInfo {
public:
    int id;
    std::vector<TransitionValue> value_list;
    DataStorage f_inp_list;
    Program* f;
    TransitionInfo(int _id, const std::vector<TransitionValue>& _value_lis, Program* _f, DataStorage&& f_inp_list);
};

class State {
public:
    DataList plan_list;
    std::vector<Transition*> in_list, out_list;
    std::vector<TransitionInfo*> full_trans_list;
    std::map<State*, Transition*> out_trans_map;
    Data state;
    State(const Data& _state);
    Transition* insertOutTransition(State* t);
    void uniquePlans();
};

class ExecutionLog {
public:
    State* start;
    std::vector<State*> state_list;
    DataList env;
    ExecutionLog(State* _start, const std::vector<State*>& _state_list, const DataList& _env);
    ~ExecutionLog();
    void print();
    int getStateNum();
    int getTransitionNum();
    int getExpansionNum();
};


#endif //DPSYNTHESISNEW_EXECUTIONLOG_H
