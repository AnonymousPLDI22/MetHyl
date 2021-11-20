//
// Created by pro on 2021/10/20.
//

#include "relation_rewriter.h"
#include "hylodp_basic.h"
#include "autolifter/autolifter.h"
#include <cassert>
#include <unordered_set>
#include "glog/logging.h"

RelationRewriter::RelationRewriter(InputSampler *_sampler, KeyEqRelation* _eq_relation, int timeout):
    sampler(_sampler->getLimitedSampler()), task(_sampler->t), state_relation(_eq_relation) {
    if (!state_relation) throw RewriterException();
    if (timeout > 0) guard = new TimeGuard(timeout);
    let_free_t = program::removeAllLet(task->t);
    for (auto* v: state_relation->key_list) {
        cared_functions.push_back(v);
    }
}

bool RelationRewriter::isValue(Program* p) {
    for (auto& v_info: value_list) {
        if (v_info.pos == p) return true;
    }
    return false;
}

int RelationRewriter::getValueIndex(Program *p, const std::vector<int> &trace) {
    for (int i = 0; i < value_list.size(); ++i) {
        if (value_list[i].pos == p && value_list[i].trace == trace) return i;
    }
    assert(0);
}

int RelationRewriter::getModIndex(Program *p, const std::vector<int> &trace) {
    for (int i = 0; i < mod_list.size(); ++i) {
        if (mod_list[i].pos == p && mod_list[i].trace == trace) return i;
    }
    assert(0);
}

Program * RelationRewriter::rewriteAllComponent(Type *t, Program *pos, std::vector<int> &trace) {
    if (t->type == T_PROD) {
        ProgramList sub_list;
        for (int i = 0; i < t->param.size(); ++i) {
            trace.push_back(i);
            sub_list.push_back(rewriteAllComponent(t->param[i], pos, trace));
            trace.pop_back();
        }
        return new ProdProgram(sub_list);
    }
    if (t->type == T_VAR) {
        int mod_id = getModIndex(pos, trace);
        if (combinator_storage[mod_id].size() == 1) return combinator_storage[mod_id][0];
        else return new ProdProgram(combinator_storage[mod_id]);
    }
    int val_id = getValueIndex(pos, trace);
    return value_list[val_id].rep;
}

Program * RelationRewriter::rewriteT(Program *p) {
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    if (sp && sp->semantics->name == "collect") {
        auto collect_info = program::unfoldCollect(p);
        int id = collect_info.first - 1;
        TypeList param_list = task->trans_types[id];
        Type* type = param_list.size() == 1 ? param_list[0] : new Type(T_PROD, param_list);
        std::vector<int> trace;
        auto* new_content = rewriteAllComponent(type, p, trace);
        return program::buildCollect(collect_info.first, new_content);
    }
    if (sp) {
        auto* ps = dynamic_cast<ParamSemantics*>(sp->semantics);
        if (ps) {
            int pre_state_num = type::unfoldProdType(task->state_type).size();
            int next_state_num = cared_functions.size();
            if (ps->id >= pre_state_num) {
                return program::buildParam(ps->id + next_state_num - pre_state_num, ps->oup_type);
            }
        }
    }
    if (isValue(p)) {
        std::vector<int> trace;
        return rewriteAllComponent(p->oup_type, p, trace);
    }
    ProgramList sub_list;
    for (auto* sub: p->getSubPrograms()) {
        sub_list.push_back(rewriteT(sub));
    }
    return p->clone(sub_list);
}

void RelationRewriter::buildNewTask() {
    Type* state_type;
    {
        TypeList content;
        for (auto* lift: cared_functions) content.push_back(lift->oup_type);
        state_type = new Type(T_PROD, content);
    }
    std::vector<std::pair<std::string, Type*>> var_list;
    {
        for (auto& env: task->env_list) var_list.emplace_back(env.name, env.type);
    }
    Type* plan_type = task->plan_type;
    Type* trans_type;
    {
        TypeList params;
        for (auto& type_list: task->trans_types) {
            if (type_list.size() == 1) params.push_back(type_list[0]);
            else params.push_back(new Type(T_PROD, type_list));
        }
        trans_type = new Type(T_SUM, params);
    }
    Program* t = rewriteT(let_free_t);
    ProgramList f_list = task->f_list;
    Program* eval = task->eval;

    if (cared_functions.size() == 1) {
        state_type = cared_functions[0]->oup_type;
    }

    TaskExampleList example_list;
    {
        for (auto& example: task->sample_example_list) {
            DataList inp_content;
            DataList env = example.env; env.push_back(example.inp);
            for (auto* cp: cared_functions) {
                inp_content.push_back(cp->run(data::unfoldProdData(example.inp), env));
            }
            Data inp = inp_content.size() == 1 ? inp_content[0] : new ProdValue(inp_content, state_type);
            example_list.emplace_back(inp, env, example.oup);
        }
    }
    new_task = new Task(state_type, var_list, plan_type, trans_type, t, f_list, eval, example_list);
}

void RelationRewriter::buildNewRelation() {
    ProgramList key_list;
    assert(cared_functions.size() >= state_relation->key_list.size());
    for (int i = 0; i < state_relation->key_list.size(); ++i) {
        auto *key = state_relation->key_list[i];
        assert(key->toString() == cared_functions[i]->toString());
        key_list.push_back(program::buildParam(i, key->oup_type));
    }
    new_relation = new KeyEqRelation(key_list);
}

void RelationRewriter::rewrite() {
    std::vector<RelationModInfo> mod_infos;
    extractModifier(let_free_t);
    try {
        if (extractCaredFunctions(let_free_t)) {
            throw RewriterException();
        }
    } catch (RewriterException& e) {
        LOG(INFO) << "State rewriter fails while extracting components";
        new_task = task;
        new_relation = state_relation;
        throw e;
    }
    LOG(INFO) << "Collected Mods" << std::endl;
    for (auto& mod_info: mod_list) {
        mod_info.print();
    }
    LOG(INFO) << "Cared Values" << std::endl;
    for (auto* value: cared_functions) {
        std::cout << value->toString() << std::endl;
    }
    LOG(INFO) << "Collected Components" << std::endl;
    for (auto& value_info: value_list) {
        value_info.print();
    }
    std::vector<autolifter::Task*> task_list;
    TypeList inp_type = type::unfoldProdType(task->state_type);
    for (auto& env_info: task->env_list) inp_type.push_back(env_info.type);
    inp_type.push_back(task->state_type);
    for (auto& mod_info: mod_list) {
        auto* g = grammar::buildDefaultGrammar(inp_type, TINT);
        for (auto* p: cared_functions) {
            g->start_symbol->rule_list.push_back(new ExtraProgramRule(p));
        }
        task_list.push_back(new autolifter::Task(mod_info.m, mod_info.F, mod_info.example_space, g, {}));
    }
    auto* lift_solver = new autolifter::AutoLifter(task_list, cared_functions, guard);
    lift_solver->synthesis();

    cared_functions = lift_solver->lifting_list;
    for (auto& f_info: value_list) f_info.updateRep(cared_functions, task);

    TypeList state_content;
    for (auto* p: cared_functions) state_content.push_back(p->oup_type);
    Type* state_type = new Type(T_PROD, state_content);
    for (int i = 0; i < mod_list.size(); ++i) {
        ProgramList combinator_list;
        for (int j = 0; j < cared_functions.size(); ++j) {
            auto* c = lift_solver->combinator_list[i][j];
            auto* res = mod_list[i].rewriteCombinator(c, task, state_type);
            combinator_list.push_back(res);
        }
        combinator_storage.push_back(combinator_list);
    }
    /*std::cout << "res " << std::endl;
    for (auto& c_list: combinator_storage) {
        for (auto* c: c_list) std::cout << c->toString() << " "; std::cout << std::endl;
    }*/
    buildNewTask();
    // new_task->print();
    buildNewRelation();
}

Executor * RelationRewriter::getNewExecutor() const {
    auto config = sampler->e->getConfig();
    config[STATE] = new_relation;
    return new Executor(new_task, config);
}