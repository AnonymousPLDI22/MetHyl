//
// Created by pro on 2021/10/24.
//

#include "incre_rewriter.h"
#include <cassert>
#include "glog/logging.h"

PlanCollectExampleSpace::PlanCollectExampleSpace(InputSampler *_sampler, Program *_f, int _id, Type* _F):
    sampler(_sampler), f(_f), f_id(_id), F(_F) {
}

void PlanModInfo::print() const {
    std::cout << F->getName() << " " << m->toString() << std::endl;
    std::cout << "Mod example space" << std::endl;
    for (int i = 0; i < 5; ++i) {
        auto* example = example_space->getExample(i);
        auto m_oup = m->run(*example);
        std::cout << "  " << data::dataList2String(*example) << " -> " << m_oup.toString() << std::endl;
    }
}

Example * PlanCollectExampleSpace::insertEnv(const Data &inp, const DataList &env) {
    DataList res;
    DataList content = data::unfoldProdData(inp);
#ifdef DEBUG
    assert(F->param.size() == content.size());
#endif
    for (int i = 0; i < content.size(); ++i) {
        if (F->param[i]->type == T_VAR) {
            DataList x = {content[i]};
            for (auto& v: env) x.emplace_back(v);
            res.emplace_back(new ProdValue(x));
        } else {
            res.push_back(content[i]);
        }
    }
    return new Example(res);
}


void PlanCollectExampleSpace::acquireMoreExamples() {
    auto* log = sampler->getLog();
    std::unordered_set<std::string> feature_set;
    for (auto* state: log->state_list) {
        for (auto* trans_info: state->full_trans_list) {
            if (trans_info->id != f_id) continue;
            for (auto& f_inp: trans_info->f_inp_list) {
                for (auto& collect_res: f->collect(f_inp, log->env)) {
                    auto* example = insertEnv(collect_res.second, log->env);
                    auto feature = data::dataList2String(*example);
                    if (feature_set.find(feature) == feature_set.end()) {
                        example_space.push_back(example);
                        feature_set.insert(feature);
                    }
                }
            }
        }
    }
    delete log;
}

namespace {
    Program* rewriteCollectContent(Program* p, Program* x, Program* y) {
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp && sp->semantics->name == "collect") {
            auto c_info = program::unfoldCollect(sp);
            if (c_info.second == x) {
                return program::buildCollect(0, y);
            } else return new EmptyProgram();
        }
        ProgramList sub_list;
        for (auto* sub: p->getSubPrograms()) {
            sub_list.push_back(rewriteCollectContent(sub, x, y));
        }
        return p->clone(sub_list);
    }
}

void PlanRewriter::insertModifier(PlanModInfo m_info) {
    mod_list.push_back(m_info);
}

void PlanRewriter::extractModifierFromCollect(int f_id, Program *p) {
    TypeList param_list;
    for (auto* t: task->trans_types[f_id]) {
        if (t->type == T_VAR) {
            param_list.push_back(task->plan_type);
        } else param_list.push_back(t);
    }
    auto used_tmps = program::extractUsedTmps(p);
    TypeList F_params = task->trans_types[f_id];
    for (auto& info: task->env_list) {
        F_params.push_back(info.type);
    }
    for (auto& tmp_info: used_tmps) {
        F_params.push_back(tmp_info->oup_type);
    }
    auto* F = new Type(T_PROD, F_params);
    Program* inp_collector;
    {
        // build collector
        ProgramList sub_list;
        for (int i = 0; i < param_list.size(); ++i) sub_list.push_back(program::buildParam(i, param_list[i]));
        for (int i = 0; i < task->env_list.size(); ++i) {
            sub_list.push_back(program::buildParam(int(param_list.size()) + i, task->env_list[i].type));
        }
        for (auto* tmp: used_tmps) sub_list.push_back(tmp);
        inp_collector = new ProdProgram(sub_list);
        inp_collector = rewriteCollectContent(normalized_f_list[f_id], p, inp_collector);
    }
    // LOG(INFO) << "collect mod from " << p->toString() << std::endl;
    // std::cout << inp_collector->toString() << std::endl;
    Program* mod;
    {
        std::map<std::string, Program*> rewrite_map;
        for (int i = 0; i < task->trans_types[f_id].size(); ++i) {
            auto* type = task->trans_types[f_id][i];
            if (type->type != T_VAR) continue;
            auto* outer = program::buildParam(i, x_type);
            rewrite_map[outer->toString()] = new AccessProgram(outer->copy(), 1);
        }
        int param_num = int(param_list.size());
        int pre_num = param_num + int(task->env_list.size());
        for (int i = 0; i < used_tmps.size(); ++i) {
            auto* outer = used_tmps[i];
            rewrite_map[outer->toString()] = program::buildParam(pre_num + i, outer->oup_type);
        }
        ProgramList m_subs = {program::rewriteProgramWithMap(p, rewrite_map)};
        for (int i = 0; i < task->env_list.size(); ++i) {
            auto* now = program::buildParam(param_num + i, task->env_list[i].type);
            m_subs.push_back(now);
        }
        mod = new ProdProgram(m_subs);
    }
    auto* example_space = new PlanCollectExampleSpace(sampler, inp_collector, f_id, F);
    insertModifier({f_id, p, mod, F, used_tmps, example_space});
}

void PlanRewriter::extractModifier(int f_id, Program *p) {
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    if (sp && sp->semantics->name == "collect") {
        extractModifierFromCollect(f_id, program::unfoldCollect(p).second);
        return;
    }
    for (auto* sub: p->getSubPrograms()) {
        extractModifier(f_id, sub);
    }
}