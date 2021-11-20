//
// Created by pro on 2021/9/15.
//

#include "execution_log.h"
#include <iostream>

namespace {
    void unique(DataList& d) {
        int now = 0;
        std::set<std::string> s;
        for (auto& v: d) {
            auto f = v.toString();
            if (s.find(f) == s.end()) {
                s.insert(f);
                d[now++] = v;
            }
        }
        d.resize(now);
    }
}

Expansion::Expansion(Transition *_trans, const Data& _plan): trans(_trans), plan(_plan) {}

void Expansion::uniquePlans() {
    unique(plan_list);
}
void State::uniquePlans() {
    unique(plan_list);
}

Transition::Transition(State *_s, State *_t): s(_s), t(_t) {}

Expansion * Transition::insertExpansion(const Data &plan) {
    auto feature = plan.toString();
    if (exp_map.count(feature)) return exp_map[feature];
    auto* exp = new Expansion(this, plan);
    exp_list.push_back(exp);
    return exp_map[feature] = exp;
}

State::State(const Data &_state): state(_state) {}

TransitionInfo::TransitionInfo(int _id, const std::vector<TransitionValue> &_value_list, Program* _f, DataStorage&& _f_inp_list):
    id(_id), value_list(_value_list), f(_f), f_inp_list(_f_inp_list) {
}

Transition * State::insertOutTransition(State *t) {
    if (out_trans_map.count(t)) return out_trans_map[t];
    auto* trans = new Transition(this, t);
    out_trans_map[t] = trans;
    out_list.push_back(trans);
    t->in_list.push_back(trans);
    return trans;
}

ExecutionLog::ExecutionLog(State* _start, const std::vector<State *> &_state_list, const DataList &_env):
    start(_start), state_list(_state_list), env(_env) {}

ExecutionLog::~ExecutionLog() {
    for (auto* s: state_list) {
        for (auto* t: s->out_list) {
            for (auto* exp: t->exp_list) delete exp;
            delete t;
        }
        for (auto* t: s->full_trans_list) {
            delete t;
        }
        delete s;
    }
}

int ExecutionLog::getStateNum() {return state_list.size();}
int ExecutionLog::getExpansionNum() {
    int num = 0;
    for (auto* s: state_list) for (auto* t: s->out_list) num += t->exp_list.size();
    return num;
}
int ExecutionLog::getTransitionNum() {
    int num = 0;
    for (auto* s: state_list) num += s->out_list.size();
    return num;
}

void ExecutionLog::print() {
    std::cout << "Execution Log For " << start->state.toString() << "@" << data::dataList2String(env) << std::endl;
    std::cout << "#State " << getStateNum() << " #Trans " << getTransitionNum() << " #Exp " << getExpansionNum() << std::endl;
    for (auto* s: state_list) {
        std::cout << "State " << s->state.toString() << std::endl;
        std::cout << "  Plans:";
        for (auto& p: s->plan_list) std::cout << " " << p.toString(); std::cout << std::endl;
        for (auto* edge: s->out_list) {
            std::cout << "  " << s->state.toString() << "->" << edge->t->state.toString() << std::endl;
            for (auto* t: edge->exp_list) {
                std::cout << "    Expand for " << t->plan.toString() << ":";
                for (auto& p: t->plan_list) std::cout << " " << p.toString();
                std::cout << std::endl;
            }
        }
    }
}