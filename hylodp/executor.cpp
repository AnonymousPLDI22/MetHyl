//
// Created by pro on 2021/9/14.
//

#include "executor.h"
#include "config.h"
#include "glog/logging.h"
#include <iostream>
#include <cassert>

Executor::Executor(Task *_task, const std::map<PreOrderUsageType, PreOrder *> &config): task(_task) {
    if (config.count(PLAN)) plan_order = config.find(PLAN)->second; else plan_order = new EmptyPreOder();
    if (config.count(STATE)) {
        state_relation = dynamic_cast<EqRelation*>(config.find(STATE)->second);
    } else state_relation = new IdRelation();
}

namespace {
    DataList getInpListForT(const Data& state) {
        DataList inp_list; auto* t = state.getType();
        if (t->type == T_PROD) {
            int n = t->param.size();
            for (int i = 0; i < n; ++i) {
                inp_list.push_back(state.accessProd(i));
            }
        } else {
            inp_list = {state};
        }
        return inp_list;
    }

    DataList getTypeForRecursion(const Data& trans, const TypeList& type_list) {
        DataList inp_list;
        if (type_list.size() == 1) {
            inp_list.push_back(trans);
        } else {
            for (int i = 0; i < type_list.size(); ++i) {
                inp_list.push_back(trans.accessProd(i));
            }
        }
        return inp_list;
    }

    DataList mergeResult(const DataStorage& separate_result, Program* f, const DataList& env) {
        // for (auto& l: separate_result) std::cout << l.size() << " "; std::cout << std::endl;
        auto merge_result = data::cartesianProduct(separate_result);
        DataList res;
        for (auto& f_inp: merge_result) {
            if (config::is_print) {
                std::cout << "trans " << data::dataList2String(f_inp) << std::endl;
                std::cout << f->toString() << std::endl;
                DataList tmp;
                for (auto& p: f->collect(f_inp, env)) tmp.push_back(p.second);
                std::cout << "res " << data::dataList2String(tmp) << std::endl;
            }
            for (auto& p: f->collect(f_inp, env)) {
                res.push_back(p.second);
            }
        }
        return res;
    }
}

DataList Executor::getAllPlanForTrans(const std::pair<int, Data> &trans, const DataList &env) {
    int id = trans.first - 1;
    auto& type_list = task->trans_types[id];
    DataList inp_list = getTypeForRecursion(trans.second, type_list);
    DataStorage separate_results;
    for (int i = 0; i < type_list.size(); ++i) {
        if (guard.isTimeOut()) return {};
        if (type_list[i]->type == T_VAR) {
            separate_results.push_back(_getAllPlan(inp_list[i], env));
        } else {
            separate_results.push_back({inp_list[i]});
        }
    }
    if (guard.isTimeOut()) return {};
    return mergeResult(separate_results, task->f_list[id], env);
}

namespace {
    std::map<std::string, DataList> result_cache;
}

DataList Executor::_getAllPlan(const Data &state, const DataList &env) {
    auto feature = state.toString();
    if (result_cache.count(feature)) return result_cache[feature];
    auto trans_list = task->t->collect(getInpListForT(state), env);
    DataList res;
    for (auto& trans: trans_list) {

        if (guard.isTimeOut()) return result_cache[feature] = {};
        auto plan_list = getAllPlanForTrans(trans, env);
        for (auto& plan: plan_list) {
            res.push_back(plan);
        }
    }
    res = plan_order->getMaximal(res, env);
    /*if (config::is_print) {
        LOG(INFO) << "plan for " << state.toString() << std::endl;
        for (auto &plan: res) std::cout << plan.toString() << " ";
        std::cout << std::endl;
    }*/
    return result_cache[feature] = res;
}

DataList Executor::getAllPlan(const Data &state, const DataList &env) {
    result_cache.clear();
    return _getAllPlan(state, env);
}

int Executor::execute(const Data &state, const DataList &env, double timeout) {
    guard.set(timeout);
    //std::cout << state.toString() << " " << data::dataList2String(env) << std::endl;
    DataList plan_list = getAllPlan(state, env);
    if (guard.isTimeOut()) return -config::KINF - 1;
    int res = -config::KINF;
    for (auto& plan: plan_list) {
        // std::cout << "plan " << plan.toString() << " " << data::dataList2String(env) << std::endl;
        res = std::max(res, task->evaluate({plan}, env));
    }
    return res;
}

namespace {
    std::map<std::string, State*> state_cache;
}



void Executor::buildTransition(State *s, const std::pair<int, Data> &trans, const DataList &env) {
    int id = trans.first - 1;
    auto& type_list = task->trans_types[id];
    DataList inp_list = getTypeForRecursion(trans.second, type_list);
    std::vector<std::pair<int, State*>> substates;
    DataStorage all_results;
    std::vector<TransitionValue> info_list;
    for (int i = 0; i < type_list.size(); ++i) {
        if (guard.isTimeOut()) return;
        if (type_list[i]->type == T_VAR) {
            auto* substate = _buildState(inp_list[i], env);
            substates.emplace_back(i, substate);
            all_results.push_back(substate->plan_list);
            info_list.emplace_back(substate);
        } else {
            info_list.emplace_back(inp_list[i]);
            all_results.push_back({inp_list[i]});
        }
    }
    DataStorage f_inp_list = data::cartesianProduct(all_results);
    auto* f = task->f_list[id];
    DataList all_merge_result;
    for (auto& f_inp: f_inp_list) {
        for (auto& res: f->collect(f_inp, env)) {
            all_merge_result.push_back(res.second);
        }
    }
    auto* trans_info = new TransitionInfo(id, info_list, task->f_list[id], std::move(f_inp_list));
    s->full_trans_list.push_back(trans_info);
    for (auto& p: all_merge_result) s->plan_list.push_back(p);
    for (auto& sub_info: substates) {
        int pos = sub_info.first; auto* t = sub_info.second;
        auto* edge = s->insertOutTransition(t);
        for (auto& t_plan: t->plan_list) {
            DataStorage separate_results;
            auto* exp = edge->insertExpansion(t_plan);
            for (int i = 0; i < type_list.size(); ++i) {
                if (i == pos) separate_results.push_back({t_plan});
                else separate_results.push_back(all_results[i]);
            }
            for (auto& res: mergeResult(separate_results, f, env)) {
                exp->plan_list.push_back(res);
            }
        }
    }
}

State * Executor::_buildState(const Data &state_val, const DataList &env) {
    auto feature = state_val.toString();
    if (state_cache.count(feature)) return state_cache[feature];
    auto* state = new State(state_val);
    state_cache[feature] = state;
    if (guard.isTimeOut()) return state;
    auto trans_list = task->t->collect(getInpListForT(state_val), env);
    for (auto& trans: trans_list)
        buildTransition(state, trans, env);
    for (auto* edge: state->out_list) {
        for (auto* exp: edge->exp_list) exp->uniquePlans();
    }
    state->uniquePlans();
    return state;
}

State * Executor::buildState(const Data &state, const DataList &env) {
    state_cache.clear();
    return _buildState(state, env);
}

ExecutionLog * Executor::getLog(const Data &state, const DataList &env, double timeout) {
    guard.set(timeout);
    auto* start = buildState(state, env);
    std::vector<State*> state_list;
    for (auto& state_info: state_cache) state_list.push_back(state_info.second);
    auto* res = new ExecutionLog(start, state_list, env);
    if (guard.isTimeOut()) {
        delete res; return nullptr;
    }
    return res;
}

std::map<PreOrderUsageType, PreOrder *> Executor::getConfig() const {
    return {{PLAN, plan_order}, {STATE, state_relation}};
}

void Executor::verifyViaSample() {
    for (auto sample: task->sample_example_list) {
        assert(execute(sample.inp, sample.env) == sample.oup.getInt());
    }
}