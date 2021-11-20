//
// Created by pro on 2021/10/15.
//

#include "incre_rewriter.h"
#include "glog/logging.h"
#include "autolifter/autolifter.h"
#include "example_space.h"
#include <cassert>
#include <unordered_set>
#include <algorithm>

void PlanRewriter::insertCaredValues(Program *p) {
    cared_values.push_back(p);
}

PlanRewriter::PlanRewriter(InputSampler *_sampler, PreOrder* _plan_order, int timeout):
    sampler(_sampler->getLimitedSampler()), plan_order(_plan_order) {
    task = sampler->t;
    if (timeout >= 0) guad = new TimeGuard(timeout);
    for (auto* f: task->f_list) {
        auto* res = program::removeAllLet(f);
        normalized_f_list.push_back(res);
    }
    auto* cmp_order = dynamic_cast<CmpPreOrder*>(plan_order);
    TypeList x_params = {task->plan_type};
    for (auto& info: task->env_list) {
        x_params.push_back(info.type);
    }
    x_type = new Type(T_PROD, x_params);
    if (cmp_order) {
        for (auto* value: cmp_order->cmp_list) insertCaredValues(value->key);
    } else {
        throw RewriterException();
    }
}

bool PlanRewriter::isValue(Program *p, int id) {
    for (auto& info: value_list) {
        if (info.pos == p && info.f_id == id) return true;
    }
    return false;
}

int PlanRewriter::getModIndex(Program* p, int id) {
    for (int i = 0; i < mod_list.size(); ++i) {
        if (mod_list[i].pos == p && mod_list[i].f_id == id) {
            return i;
        }
    }
    return -1;
}

PlanValueInfo PlanRewriter::getValueInfo(Program *p, std::vector<int> &trace, int id) {
    for (auto& info: value_list) {
        if (info.pos == p && info.f_id == id && info.trace == trace) return info;
    }
    assert(0);
}

Program * PlanRewriter::rewriteForValue(Type* t, Program *p, int f_id, std::vector<int> &trace) {
    if (t->type == T_PROD) {
        ProgramList sub_list;
        for (int i = 0; i < t->param.size(); ++i) {
            trace.push_back(i);
            sub_list.push_back(rewriteForValue(t->param[i], p, f_id, trace));
            trace.pop_back();
        }
        return new ProdProgram(sub_list);
    }
    auto value_info = getValueInfo(p, trace, f_id);
    return value_info.rep;
}

Program* PlanRewriter::rewriteF(Program *p, int f_id) {
    if (isValue(p, f_id)) {
        std::vector<int> trace;
        return rewriteForValue(p->oup_type, p, f_id, trace);
    }
    int index = getModIndex(p, f_id);
    if (index != -1) {
        if (combinator_storage[index].size() == 1) return combinator_storage[index][0];
        return new ProdProgram(combinator_storage[index]);
    }
    ProgramList sub_list;
    for (auto* sub: p->getSubPrograms()) {
        sub_list.push_back(rewriteF(sub, f_id));
    }
    return p->clone(sub_list);
}

namespace {
    Program* removeDummyProduct(Program* p, const TypeList& type_list, Type* pre) {
        std::map<std::string, Program*> rewrite_map;
        assert(pre->type == T_PROD && pre->param.size() == 1);
        auto* after = pre->param[0];
        for (int i = 0; i < type_list.size(); ++i) {
            if (type_list[i]->type == T_VAR) {
                auto* pp = program::buildParam(i, after);
                auto* ap = new AccessProgram(program::buildParam(i, pre), 1);
                rewrite_map[ap->toString()] = pp;
            }
        }
        return program::rewriteProgramWithMap(p, rewrite_map);
    }
}

void PlanRewriter::buildNewTask() {
    auto* new_state_type = task->state_type;
    std::vector<std::pair<std::string, Type*>> new_vars;
    {
        for (auto& env: task->env_list) {
            new_vars.emplace_back(env.name, env.type);
        }
    }
    Type* new_plan_type;
    {
        TypeList contents;
        for (auto* v: cared_values) contents.push_back(v->oup_type);
        new_plan_type = new Type(T_PROD, contents);
    }
    Type* new_trans_type;
    {
        TypeList content;
        for (auto& t_type: task->trans_types) {
            content.push_back(new Type(T_PROD, t_type));
        }
        new_trans_type = new Type(T_SUM, content);
    }
    auto* new_t = task->t;
    ProgramList new_f_list;
    {
        for (int i = 0; i < normalized_f_list.size(); ++i) {
            new_f_list.push_back(program::removeProdAccess(rewriteF(normalized_f_list[i], i)));
        }
    }
    Program* new_eval;
    {
        int eval_ind = -1;
        for (int i = 0; i < cared_values.size(); ++i) {
            if (cared_values[i]->toString() == task->eval->toString()) eval_ind = i;
        }
        assert(eval_ind != -1);
        new_eval = new AccessProgram(program::buildParam(0, new_plan_type), eval_ind + 1);
    }
    auto new_example_list = task->sample_example_list;
    if (cared_values.size() == 1) {
        for (int i = 0; i < new_f_list.size(); ++i) {
            new_f_list[i] = removeDummyProduct(new_f_list[i], task->trans_types[i], new_plan_type);
        }
        new_plan_type = cared_values[0]->oup_type;
        new_eval = program::buildParam(0, new_plan_type);
    }
    new_task = new Task(new_state_type, new_vars, new_plan_type, new_trans_type, new_t, new_f_list, new_eval, new_example_list);
}

void PlanRewriter::buildNewOrder() {
    auto* cmp_order = dynamic_cast<CmpPreOrder*>(plan_order);
    if (cared_values.size() == 1) {
        assert(cmp_order->cmp_list.size() == 1);
        auto* cmp = cmp_order->cmp_list[0];
        auto* key = cmp->key;
        assert(key->toString() == cared_values[0]->toString());
        new_order = new CmpPreOrder({new CmpProgram(cmp->type, program::buildParam(0, key->oup_type))});
    } else {
        assert(cmp_order->cmp_list.size() <= cared_values.size());
        TypeList plan_content;
        for (auto* v: cared_values) plan_content.push_back(v->oup_type);
        auto* plan_type = new Type(T_PROD, plan_content);
        ProgramList cmp_list;
        for (int i = 0; i < cmp_order->cmp_list.size(); ++i) {
            auto* cmp = cmp_order->cmp_list[i];
            auto* key = cmp->key;
            assert(key->toString() == cared_values[i]->toString());
            cmp_list.push_back(new CmpProgram(cmp->type,
                    new AccessProgram(program::buildParam(0, plan_type), i + 1)));
        }
        new_order = new CmpPreOrder(cmp_list);
    }
}

void PlanRewriter::rewrite() {
    for (int f_id = 0; f_id < task->trans_types.size(); ++f_id) {
        extractModifier(f_id, normalized_f_list[f_id]);
    }
    /*LOG(INFO) << "Plan Mod List";
    for (auto& m_info: mod_list) {
        m_info.print();
    }*/
    try {
        for (int f_id = 0; f_id < task->trans_types.size(); ++f_id) {
            if (extractValues(f_id, normalized_f_list[f_id])) {
                throw RewriterException();
            }
        }
    } catch (RewriterException& e) {
        LOG(INFO) << "Plan rewriter fails while extracting components";
        new_task = task;
        new_order = plan_order;
        throw e;
    }
    /*LOG(INFO) << "Cared Value List";
    for (auto* v: cared_values) {
        std::cout << "  " << v->toString() << std::endl;
    }
    LOG(INFO) << "Plan Value List";
    for (auto& v_info: value_list) {
        v_info.print();
    }*/

    std::vector<autolifter::Task*> task_list;
    TypeList value_inp_type = {task->plan_type};
    for (auto& env: task->env_list) value_inp_type.push_back(env.type);
    for (auto& m_info: mod_list) {
        Grammar* g = grammar::buildDefaultGrammar(value_inp_type, TINT);
        task_list.push_back(autolifter::buildTask(m_info.m, m_info.F, m_info.example_space, g));
    }
    auto* solver = new autolifter::AutoLifter(task_list, cared_values, guad);
    solver->synthesis();
    // solver->printResult();

    for (auto& v_info: value_list) v_info.getFullResult(cared_values, task);
    cared_values = solver->lifting_list;
    for (int i = 0; i < task_list.size(); ++i) {
        ProgramList c_list;
        for (int j = 0; j < cared_values.size(); ++j) {
            auto* res = solver->combinator_list[i][j];
            int f_id = mod_list[i].f_id;
            int pre_num = int(task->trans_types[f_id].size()) + int(task->env_list.size());
            // std::cout << "c " << i << " " << j << " " << res->toString() << std::endl;
            ProgramList rewrite_list;
            for (int k = 0; k < pre_num; ++k) rewrite_list.push_back(nullptr);
            for (auto* t: mod_list[i].tmp_list) rewrite_list.push_back(t);
            c_list.push_back(program::rewriteParams(res, rewrite_list));
        }
        combinator_storage.push_back(c_list);
    }

    LOG(INFO) << "cared values " << std::endl;
    for (auto* c: cared_values) std::cout << "  " << c->toString() << std::endl;
    /*for (auto& c_list: combinator_storage) {
        for (auto* c: c_list) std::cout << c->toString() << " "; std::cout << std::endl;
    }*/
    buildNewOrder();
    buildNewTask();
    /*new_task->print(); auto* e = new Executor(new_task);
    auto example = new_task->sample_example_list[0];
    config::is_print = true;
    std::cout << e->execute(example.inp, example.env) << std::endl;
    config::is_print = false;*/
}