//
// Created by pro on 2021/10/24.
//

#include "incre_rewriter.h"
#include "polygen/polygen.h"
#include <cassert>

void PlanValueInfo::print() const {
    std::cout << pos->toString();
    for (int i: trace) std::cout << "." << i + 1;
    std::cout << " " << rep->toString() << std::endl;
}

namespace {
    Program* rewriteWithFullType(Program* p, Type* type, const TypeList& type_list) {
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp) {
            auto* ps = dynamic_cast<ParamSemantics*>(sp->semantics);
            if (ps && ps->id < type_list.size() && type_list[ps->id]->type == T_VAR) {
                return program::buildParam(ps->id, type);
            }
        }
        ProgramList sub_list;
        for (auto* sub: p->getSubPrograms()) {
            sub_list.push_back(rewriteWithFullType(sub, type, type_list));
        }
        return p->clone(sub_list);
    }
}

void PlanValueInfo::getFullResult(const ProgramList& cared_list, Task* task) {
    TypeList res;
    for (auto* p: cared_list) res.push_back(p->oup_type);
    rep = rewriteWithFullType(rep, new Type(T_PROD, res), task->trans_types[f_id]);
}

namespace {
    class PlanValueExampleSpace: public PointExampleSpace {
    public:
        InputSampler* sampler;
        Program* collector;
        Program* oup_program;
        ProgramList inp_programs;
        int f_id;

        PlanValueExampleSpace(InputSampler* _sampler, Program* _c, Program* _oup_program, const ProgramList& _inp_list, int _id):
            sampler(_sampler), collector(_c), oup_program(_oup_program), inp_programs(_inp_list), f_id(_id) {
        }

        PointExample* buildExample(const DataList& f_inp, const DataList& tmp_value_list, const DataList& env) {
            auto& type_list = sampler->t->trans_types[f_id];
#ifdef DEBUG
            assert(type_list.size() == f_inp.size());
#endif
            DataList inp;
            for (int i = 0; i < f_inp.size(); ++i) {
                if (type_list[i]->type == T_VAR) {
                    DataList content;
                    for (auto* p: inp_programs) content.push_back(p->run({f_inp[i]}, env));
                    inp.emplace_back(new ProdValue(content));
                } else {
                    inp.emplace_back(f_inp[i]);
                }
            }
            for (auto& v: env) inp.push_back(v);
            for (auto& v: tmp_value_list) inp.push_back(v);

            DataList inp_for_oup = data::mergeDataList(f_inp, data::mergeDataList(env, tmp_value_list));
            Data oup = oup_program->run(inp_for_oup);
            // std::cout << data::dataList2String(f_inp) << " " << data::dataList2String(tmp_value_list) << " " << data::dataList2String(env) << std::endl;
            // std::cout << "  -> " << example::pointExample2String({inp, oup}) << std::endl;
            return new PointExample(inp, oup);
        }

        virtual void acquireMoreExamples() {
            auto* log = sampler->getLog();
            std::unordered_set<std::string> feature_set;
            for (auto* state: log->state_list) {
                for (auto* t_info: state->full_trans_list) {
                    if (t_info->id != f_id) continue;
                    for (auto& f_inp: t_info->f_inp_list) {
                        for (auto collect_res: collector->collect(f_inp, log->env)) {
                            auto* current_example = buildExample(f_inp, data::unfoldProdData(collect_res.second), log->env);
                            auto feature = example::pointExample2String(*current_example);
                            if (feature_set.find(feature) == feature_set.end()) {
                                feature_set.insert(feature);
                                example_space.push_back(current_example);
                            }
                        }
                    }
                }
            }
            delete log;
        }
    };

    bool getTraceForSubprogram(Program* now, Program* x, ProgramList& trace) {
        trace.push_back(now);
        if (now == x) return true;
        for (auto* sub: now->getSubPrograms()) {
            if (getTraceForSubprogram(sub, x, trace)) return true;
        }
        trace.pop_back();
        return false;
    }

    Program* collectMidValue(Program* root, Program* x, Program* y) {
        ProgramList trace;
        assert(getTraceForSubprogram(root, x, trace));
        Program* change_pos = trace[0];
        for (int i = 1; i < trace.size(); ++i) {
            auto* pre = trace[i - 1], *now = trace[i];
            auto* ip = dynamic_cast<IfProgram*>(pre);
            if (ip && (now == ip->tb || now == ip->fb)) change_pos = now;
            auto* fp = dynamic_cast<ForEachProgram*>(pre);
            if (fp && now == fp->content) change_pos = now;
        }
        return program::rewriteCollect(root, change_pos, y);
    }

    // TODO: analysis not only from the syntax
    int getPlanId(Program* f, const TypeList& type_list) {
        auto* sf = dynamic_cast<SemanticsProgram*>(f);
        if (sf) {
            auto* ps = dynamic_cast<ParamSemantics*>(sf->semantics);
            if (ps && ps->id < type_list.size()) {
                if (type_list[ps->id]->type == T_VAR) return ps->id;
                throw RewriterException();
            }
        }
        int id = -1;
        for (auto* sub: f->getSubPrograms()) {
            int res = getPlanId(sub, type_list);
            if (res == -1) continue;
            if (id == -1) id = res;
            else if (id != res) throw RewriterException();
        }
        return id;
    }
}

void PlanRewriter::extractCaredComponents(int f_id, Program* p) {
    auto component_info = program::extractAllComponents(p->oup_type, p);
    auto used_tmps = program::extractUsedTmps(p);
    TypeList type_list = task->trans_types[f_id];
    Program* tmp_collector = new ProdProgram(used_tmps);
    Program* inp_collector = collectMidValue(normalized_f_list[f_id], p, tmp_collector);

    std::map<std::string, Program*> inner_rewrite_map;
    int pre_num = int(type_list.size()) + int(task->env_list.size());
    for (int i = 0; i < used_tmps.size(); ++i) {
        auto* tmp = used_tmps[i];
        inner_rewrite_map[tmp->toString()] = program::buildParam(pre_num + i, tmp->oup_type);
    }
    auto inner_rewriter = [=](Program* p)->Program*{
        return program::rewriteProgramWithMap(p, inner_rewrite_map);
    };
    ProgramList param_rewrite_list;
    for (int i = 0; i < pre_num; ++i) param_rewrite_list.push_back(nullptr);
    for (auto* tmp: used_tmps) param_rewrite_list.push_back(tmp);
    auto outer_rewriter = [=](Program* p)->Program*{
        return program::rewriteParams(p, param_rewrite_list);
    };

    for (auto& component: component_info) {
        auto* inner = inner_rewriter(component.program);
        auto* example_space = new PlanValueExampleSpace(sampler, inp_collector, inner, cared_values, f_id);
        // std::cout << "overview example space " << f_id << " " << component.program->toString() << std::endl;
        auto* solver = polygen::buildDefaultPolyGen();
        Program* rep = nullptr;
        try {
            rep = solver->synthesis(example_space, new TimeGuard(config::KPolyGenTimeOut));
        } catch (TimeOutError& e) {
        }
        delete solver;
        delete example_space;
        if (!rep) {
            if (!used_tmps.empty()) throw RewriterException();
            int plan_id = getPlanId(p, type_list);
            ProgramList normalize_list;
            for (int i = 0; i < type_list.size(); ++i) {
                if (i == plan_id) {
                    normalize_list.push_back(program::buildParam(0, task->plan_type));
                } else normalize_list.push_back(nullptr);
            }
            for (int i = 0; i < task->env_list.size(); ++i) {
                normalize_list.push_back(program::buildParam(i + 1, task->env_list[i].type));
            }
            Program* v = program::rewriteParams(component.program, normalize_list);
            cared_values.push_back(v);
            Type *cared_type;
            {
                TypeList cared_params;
                for (auto *value: cared_values) cared_params.push_back(value->oup_type);
                cared_type = new Type(T_PROD, cared_params);
            }
            rep = program::buildParam(plan_id, cared_type);
            rep = new AccessProgram(rep, int(cared_values.size()));
        }
        value_list.push_back({f_id, p, component.trace, outer_rewriter(rep)});
    }
}

bool PlanRewriter::extractValues(int f_id, Program* p) {
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    bool is_cared = false;
    if (sp) {
        if (sp->semantics->name == "collect") return false;
        auto* ps = dynamic_cast<ParamSemantics*>(sp->semantics);
        if (ps) {
            int id = ps->id; auto& type_list = task->trans_types[f_id];
            if (id < type_list.size() && type_list[id]->type == T_VAR) is_cared = true;
        }
    }
    for (auto* sub: p->getSubPrograms()) {
        is_cared |= extractValues(f_id, sub);
    }
    if (is_cared && type::isIntProduct(p->oup_type)) {
        extractCaredComponents(f_id, p);
        return false;
    }
    return is_cared;
}