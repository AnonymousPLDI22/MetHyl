//
// Created by pro on 2021/10/22.
//

#include "input_sampler.h"
#include "relation_rewriter.h"
#include "hylodp_basic.h"
#include <cassert>
#include <unordered_set>
#include "glog/logging.h"
#include "polygen/polygen.h"

void RelationValueInfo::updateRep(const ProgramList &cared_values, Task* task) {
    ProgramList rewrite_list;
    int param_id = 0;
    {
        ProgramList content;
        for (auto* cv: cared_values) {
            content.push_back(program::buildParam(param_id++, cv->oup_type));
        }
        rewrite_list.push_back(new ProdProgram(content));
    }
    for (auto& var_info: task->env_list) {
        rewrite_list.push_back(program::buildParam(param_id++, var_info.type));
    }
    rewrite_list.push_back(program::buildParam(param_id++, task->state_type));
    rep = program::rewriteParams(rep, rewrite_list);
    rep = program::removeProdAccess(rep);
}

namespace {

    bool getTrace(Program* p, Program* x, ProgramList& trace) {
        trace.push_back(p);
        if (p == x) return true;
        for (auto* sub: p->getSubPrograms()) {
            if (getTrace(sub, x, trace)) return true;
        }
        trace.pop_back();
        return false;
    }

    Program* insertCollect(Program* root, Program* x, Program* y) {
        ProgramList trace;
        getTrace(root, x, trace);
        assert(!trace.empty());
        auto* change_pos = trace[0];
        for (int i = 1; i < trace.size(); ++i) {
            auto* pre = trace[i - 1];
            auto* ipre = dynamic_cast<IfProgram*>(pre);
            if (ipre && (trace[i] == ipre->fb || trace[i] == ipre->tb)) change_pos = trace[i];
            auto* fpre = dynamic_cast<ForEachProgram*>(pre);
            if (fpre && trace[i] == fpre->content) change_pos = trace[i];
        }
        return program::rewriteCollect(root, change_pos, y);
    }
}

void RelationValueInfo::print() const {
    std::cout << "rewrite (" << pos->toString() << ")";
    for (int i: trace) std::cout << "." << i + 1;
    std::cout << " " << rep->toString() << std::endl;
}

namespace {
    struct RelationValueRepExampleSpace: public PointExampleSpace {
    public:
        InputSampler* sampler;
        Program* oup_program;
        ProgramList inp_programs;
        Program* t;
        RelationValueRepExampleSpace(InputSampler* _sampler, Program* _oup_program, ProgramList& _inp_lists, Program* _t):
            sampler(_sampler), oup_program(_oup_program), inp_programs(_inp_lists), t(_t) {
        }
        PointExample* buildExample(const DataList& state_list, const DataList& env, const Data& start, const DataList& tmp_list) {
            DataList full_env = env; full_env.push_back(start);
            DataList inp;
            {
                DataList f_content;
                for (auto* ip: inp_programs) {
                    try {
                        f_content.push_back(ip->run(state_list, full_env));
                    } catch (SemanticsError& e) {
                        f_content.push_back(std::rand() & 1);
                    }
                }
                inp.emplace_back(new ProdValue(f_content));
                for (auto& v: full_env) inp.push_back(v);
                for (auto& v: tmp_list) inp.push_back(v);
            }
            DataList inp_for_oup = data::mergeDataList(state_list, data::mergeDataList(full_env, tmp_list));
            // std::cout << data::dataList2String(inp_for_oup) << std::endl;
            Data oup = oup_program->run(inp_for_oup);
            return new PointExample(inp, oup);
        }
        virtual void acquireMoreExamples() {
            auto* log = sampler->getLog();
            auto env = log->env; auto start_state = log->start->state;
            std::unordered_set<std::string> feature_set;
            for (auto* state: log->state_list) {
                auto t_inp = data::unfoldProdData(state->state);
                auto collect_res = t->collect(t_inp, env);

                DataList c_list;
                for (auto& c_res: collect_res) c_list.push_back(c_res.second);
                //std::cout << data::dataList2String(c_list) << " " << data::dataList2String(t_inp) << " " << oup_program->toString() << std::endl;

                for (auto& c_res: collect_res) {
                    auto tmp_list = data::unfoldProdData(c_res.second);
                    auto* p_example = buildExample(t_inp, env, start_state, tmp_list);
                    auto feature = example::pointExample2String(*p_example);
                    if (feature_set.find(feature) == feature_set.end()) {
                        feature_set.insert(feature);
                        example_space.push_back(p_example);
                    } else delete p_example;
                }
            }
            delete log;
        }

    };
}

void RelationRewriter::extractCaredComponentFromFunctions(Program *p) {
    //std::cout << "cared " << p->toString() << std::endl;
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    assert((sp&&sp->semantics->name == "collect") || type::isIntProduct(p->oup_type));
    auto tmp_list = program::extractUsedTmps(p);
    Program* t_collector = new ProdProgram(tmp_list);
    t_collector = insertCollect(let_free_t, p, t_collector);

    std::vector<program::ComponentInfo> cared_components;
    if (sp && sp->semantics->name == "collect") {
        auto collect_info = program::unfoldCollect(sp);
        int tid = collect_info.first - 1;
        Program* content = collect_info.second;
        TypeList t_type = task->trans_types[tid];
        cared_components = program::extractAllComponents(t_type.size() == 1 ? t_type[0] : new Type(T_PROD, t_type), content);
    } else cared_components = program::extractAllComponents(p->oup_type, p);

    int state_param_num = int(type::unfoldProdType(task->state_type).size());

    std::map<std::string, Program*> tmp_rewrite_map;
    {
        int param_num = state_param_num + int(task->env_list.size()) + 1;
        for (auto* tmp: tmp_list) {
            tmp_rewrite_map[tmp->toString()] = program::buildParam(param_num++, tmp->oup_type);
        }
    }
    ProgramList tmp_rewrite_list;
    {
        int pre_num = 2 + int(task->env_list.size());
        for (int i = 0; i < pre_num; ++i) tmp_rewrite_list.push_back(nullptr);
        for (auto* t: tmp_list) tmp_rewrite_list.push_back(t);
    }

    for (auto& component: cared_components) {
        if (component.type->type == T_VAR) continue;
        if (component.type->type == T_VOID) {
            value_list.push_back({p, component.trace, program::buildConst(Data())});
            continue;
        }
        if (component.type->type == T_INT) {
            component.program = program::removeProdAccess(component.program);
            if (program::isConstant(component.program)) {
                value_list.push_back({p, component.trace, component.program});
                continue;
            }
            auto *inner = program::rewriteProgramWithMap(component.program, tmp_rewrite_map);
            auto* example_space = new RelationValueRepExampleSpace(sampler, inner, cared_functions, t_collector);
            // std::cout << inner->toString() << std::endl;
            polygen::PolyGenConfig c;
            if (sp && sp->semantics->name == "collect") c.insertComponent(sp->sub_list[1]);
            else c.insertComponent(p);
            if (program::isRef(p)) c.isRef = true;
            auto* solver = new polygen::PolyGen(c);
            Program* rep = nullptr;
            try {
                rep = solver->synthesis(example_space, new TimeGuard(config::KPolyGenTimeOut));
            } catch (TimeOutError& e){
            }
            //std::cout << "finished" << std::endl;
            if (!rep) {
                //std::cout << component.program->toString() << std::endl;
                if (!program::extractUsedTmps(component.program).empty()) {
                    throw RewriterException();
                }
                //std::cout << "new " << component.program->toString() << std::endl;
                cared_functions.push_back(component.program);
                TypeList x_content;
                for (auto* cf: cared_functions) x_content.push_back(cf->oup_type);
                auto* x_type = new Type(T_PROD, x_content);
                rep = new AccessProgram(program::buildParam(0, x_type), cared_functions.size());
            }
            rep = program::rewriteParams(rep, tmp_rewrite_list);
            // std::cout << component.program->toString() << " " << rep->toString() << std::endl;
            value_list.push_back({p, component.trace, rep});
            continue;
        }
        LOG(INFO) << "Unsupported basic type of values " << component.type->getName();
        throw RewriterException();
    }
}

bool RelationRewriter::extractCaredFunctions(Program *p) {
    // std::cout << p->toString() << std::endl;
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    bool is_cared = false;
    if (sp) {
        if (sp->semantics->name == "collect") {
            extractCaredComponentFromFunctions(p);
            return false;
        }
        auto* ps = dynamic_cast<ParamSemantics*>(sp->semantics);
        if (ps) {
            int state_num = task->state_type->type == T_PROD ? int(task->state_type->param.size()) : 1;
            if (ps->id < state_num) is_cared = true;
        }
    }
    for (auto* sub: p->getSubPrograms()) {
        if (extractCaredFunctions(sub)) is_cared = true;
    }
    if (is_cared && type::isIntProduct(p->oup_type)) {
        extractCaredComponentFromFunctions(p);
        return false;
    }
    // std::cout << p->toString() << " " << is_cared << std::endl;
    return is_cared;
}