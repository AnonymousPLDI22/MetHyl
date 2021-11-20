//
// Created by pro on 2021/10/22.
//

#include "input_sampler.h"
#include "relation_rewriter.h"
#include "hylodp_basic.h"
#include <cassert>
#include <unordered_set>
#include "glog/logging.h"

void RelationRewriter::insertModInfo(const RelationModInfo &info) {
    mod_list.push_back(info);
}

void RelationModInfo::print() const {
    std::cout << F->getName() << " " << m->toString() << std::endl;
}

Program * RelationModInfo::rewriteCombinator(Program *c, Task *t, Type* state_type) {
    ProgramList rewrite_list;
    int inp_id = 0;
    {
        TypeList new_state = type::unfoldProdType(state_type);
        ProgramList content;
        for (auto* type: new_state) content.push_back(program::buildParam(inp_id++, type));
        rewrite_list.push_back(new ProdProgram(content));
    }
    for (auto& var: t->env_list) rewrite_list.push_back(program::buildParam(inp_id++, var.type));
    rewrite_list.push_back(program::buildParam(inp_id++, t->state_type));
    for (auto* tmp: used_tmps) rewrite_list.push_back(tmp);
    auto* res = program::rewriteParams(c, rewrite_list);
    return program::removeProdAccess(res);
}

namespace {
    struct RelationModExampleSpace: public ExampleSpace {
        InputSampler* sampler;
        Program* t;
        Type* x_type;
        RelationModExampleSpace(InputSampler* _sampler, Program* _t, Type* _x_type):
            sampler(_sampler), t(_t), x_type(_x_type) {}
        virtual void acquireMoreExamples() {
            std::unordered_set<std::string> feature_set;
            auto* log = sampler->getLog();
            auto env = log->env; Data start_state = log->start->state;
            for (auto* state: log->state_list) {
                DataList t_inp = data::unfoldProdData(state->state);
                for (auto& collect_res: t->collect(t_inp, env)) {
                    DataList example;
                    {
                        DataList content = data::mergeDataList(t_inp, env);
                        content.push_back(start_state);
                        example.emplace_back(new ProdValue(content, x_type));
                    }
                    example = data::mergeDataList(example, env);
                    example.push_back(start_state);
                    example = data::mergeDataList(example, data::unfoldProdData(collect_res.second));
                    auto feature = data::dataList2String(example);
                    if (feature_set.find(feature) == feature_set.end()) {
                        feature_set.insert(feature);
                        example_space.push_back(new Example(example));
                    }
                }
            }
            delete log;
        }
    };
}

void RelationRewriter::extractModFromCollect(Program* cp) {
    // extract id
    auto c_info = program::unfoldCollect(cp);
    int id = c_info.first - 1; auto* content = c_info.second;
    TypeList type_list = task->trans_types[id];
    TypeList state_params = type::unfoldProdType(task->state_type);
    auto components = program::extractAllComponents(type_list.size() == 1 ? type_list[0] : new Type(T_PROD, type_list), content);
    int now = 0;
    for (auto& info: components) {
        if (info.type->type != T_VAR) continue;
        components[now++] = info;
    }
    components.resize(now);
    if (components.empty()) return;

    Type* x_type;
    {
        TypeList x_content = state_params;
        for (auto &info: task->env_list) x_content.push_back(info.type);
        x_content.push_back(task->state_type);
        x_type = new Type(T_PROD, x_content);
    }
    auto used_tmps = program::extractUsedTmps(cp);
    Type* F;
    {
        TypeList F_content = {TVARA};
        for (auto &info: task->env_list) F_content.push_back(info.type);
        for (auto& tmp: used_tmps) F_content.push_back(tmp->oup_type);
        F_content.push_back(task->state_type);
        F = new Type(T_PROD, F_content);
    }
    auto* collector = new ProdProgram(used_tmps);
    auto* t_collector = program::rewriteCollect(let_free_t, cp, collector);
    auto* example_space = new RelationModExampleSpace(sampler, t_collector, x_type);

    std::map<std::string, Program*> mod_rewrite_map;
    {
        for (int i = 0; i < state_params.size(); ++i) {
            auto* outer = program::buildParam(i, state_params[i]);
            auto* inner = new AccessProgram(program::buildParam(0, x_type), i + 1);
            mod_rewrite_map[outer->toString()] = inner;
        }
        int state_num = int(state_params.size());
        for (int i = 0; i < task->env_list.size(); ++i) {
            auto* outer = program::buildParam(state_num + i, task->env_list[i].type);
            auto* inner = program::buildParam(1 + i, task->env_list[i].type);
            mod_rewrite_map[outer->toString()] = inner;
        }
        int pre_num = 1 + 1 + int(task->env_list.size());
        for (int i = 0; i < used_tmps.size(); ++i) {
            mod_rewrite_map[used_tmps[i]->toString()] = program::buildParam(pre_num + i, used_tmps[i]->oup_type);
        }
    }

    for (auto& info: components) {
        auto* mod = program::rewriteProgramWithMap(info.program, mod_rewrite_map);
        ProgramList full_content;
        if (task->state_type->type == T_PROD) {
            for (int i = 0; i < state_params.size(); ++i) full_content.push_back(new AccessProgram(mod->copy(), i + 1));
        } else {
            full_content.push_back(mod);
        }
        for (int i = 0; i < task->env_list.size(); ++i) {
            full_content.push_back(program::buildParam(i + 1, task->env_list[i].type));
        }
        full_content.push_back(program::buildParam(int(task->env_list.size()) + 1, task->state_type));
        Program* full_mod = new ProdProgram(full_content);
        full_mod = program::removeProdAccess(full_mod);
        insertModInfo({cp, info.trace, full_mod, F, used_tmps, example_space});
    }
}

void RelationRewriter::extractModifier(Program* p) {
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    if (sp) {
        if (sp->semantics->name == "collect") {
            extractModFromCollect(sp);
            return;
        }
    }
    for (auto* sub: p->getSubPrograms()) {
        extractModifier(sub);
    }
}