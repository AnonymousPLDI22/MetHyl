//
// Created by pro on 2020/12/2.
//

#include "autolifter.h"
#include "config.h"
#include "glog/logging.h"
#include <set>
#include <cassert>

using namespace autolifter;

AutoLifter::AutoLifter(const std::vector<Task *>& _task_list, const ProgramList& init_list, TimeGuard* _guard):
    task_list(_task_list), lifting_list(init_list), combinator_list(task_list.size()), guard(_guard) {
}

int AutoLifter::getLiftingId(Program *program) {
    auto name = program->toString();
    for (int i = 0; i < lifting_list.size(); ++i) {
        if (lifting_list[i]->toString() == name) return i;
        bool is_equal = true;
        for (auto* task: task_list) {
            if (!task->checkFEqual(program, lifting_list[i])) is_equal = false;
        }
        if (is_equal) return i;
    }
    lifting_list.push_back(program);
    /*for (auto* t: task_list) {
        t->g->start_symbol->rule_list.push_back(new ExtraProgramRule(program));
    }*/
    return int(lifting_list.size()) - 1;
}

namespace {
    Program* rewriteParamForCombinator(Program* program, const std::vector<bool>& is_plan, const std::vector<int>& id_list, Type* x_type) {
        auto* ap = dynamic_cast<AccessProgram*>(program);
        if (ap) {
            auto* sp = dynamic_cast<SemanticsProgram*>(ap->s);
            if (sp) {
                auto* ps = dynamic_cast<ParamSemantics*>(sp->semantics);
                if (ps && is_plan[ps->id]) {
                    return new AccessProgram(program::buildParam(ps->id, x_type), id_list[ap->ind] + 1);
                }
            }
        }
        ProgramList sub_list;
        for (auto* sub: program->getSubPrograms()) {
            sub_list.push_back(rewriteParamForCombinator(sub, is_plan, id_list, x_type));
        }
        return program->clone(sub_list);
    }
}

Program * AutoLifter::rewriteProgram(const TmpCombinatorInfo &info, Task *task, Type* x_type) {
    std::vector<bool> is_plan;
    auto* F = task->cache->F;
    for (auto* sub_type: F->param) {
        is_plan.push_back(sub_type->type == T_VAR);
    }
    auto* res = rewriteParamForCombinator(info.program, is_plan, info.inp_list, x_type);
    return res;
}

void AutoLifter::synthesis() {
    std::unordered_map<std::string, int> existing_programs;
    std::vector<std::vector<TmpCombinatorInfo>> tmp_info_list;

    for (int pos = 0; pos < lifting_list.size(); ++pos) {
        tmp_info_list.emplace_back();
        auto& tmp = tmp_info_list[pos];
        auto* p = lifting_list[pos];
        for (auto* task: task_list) {
            if (guard && guard->isTimeOut()) throw TimeOutError();
            task->setTarget(p);
            auto* sf = new SfSolver(task, guard);
            std::vector<int> id_list = {pos};
            LOG(INFO) << "AutoLifter rewrite " << task->p->toString() << " " << task->cache->m->toString() << std::endl;

            //std::cout << "sf " << p->toString() << " " << task->cache->m->toString() << std::endl;
            auto sf_res = sf->synthesis(lifting_list);
            std::cout << "fin " << sf_res.size() << " " << program::programList2String(sf_res) << std::endl;
            for (auto* new_lifting: sf_res) {
                id_list.push_back(getLiftingId(new_lifting));
            }

            if (!type::isIntVarProduct(task->cache->F)) {
                id_list = {pos};
                sf_res.clear();
                for (int i = 0; i < lifting_list.size(); ++i)
                    if (i != pos) {
                        if (!task->isCached(lifting_list[i])) task->cache->insertCache(lifting_list[i], {}, {});
                        id_list.push_back(i);
                        sf_res.push_back(lifting_list[i]);
                    }
            }

            auto* sc = new ScSolver(task, sf_res, guard);
            /*LOG(INFO) << "AutoLifter rewrite " << task->p->toString() << std::endl;
            std::cout << "cared list" << std::endl;
            for (auto* res: sf_res) {
                std::cout << "  " << res->toString() << std::endl;
            }*/
            auto* c = sc->synthesis();
            std::cout << "res " << c->toString() << std::endl;
            tmp.emplace_back(id_list, c);
            delete sf;
            delete sc;
        }
    }

    //LOG(INFO) << "Liftings";
    //for (auto* c: lifting_list) LOG(INFO) << c->toString();
    TypeList x_content;
    for (auto* lp: lifting_list) x_content.push_back(lp->oup_type);
    auto* x_type = new Type(T_PROD, x_content);
    for (auto& tmp_res: tmp_info_list) {
        for (int i = 0; i < tmp_res.size(); ++i) {
            auto* task = task_list[i];
            auto* res = rewriteProgram(tmp_res[i], task, x_type);
            combinator_list[i].push_back(res);
            // std::cout << tmp_res[i].inp_list.size() << " " << tmp_res[i].program->toString() << " " << res->toString() << std::endl;
        }
    }
}

void AutoLifter::printResult() const {
    std::cout << "Values " << std::endl;
    for (auto* p: lifting_list) {
        std::cout << "  " << p->toString() << std::endl;
    }
    for (int i = 0; i < combinator_list.size(); ++i) {
        std::cout << "Combinator for task " << i << std::endl;
        for (auto* p: combinator_list[i]) {
            std::cout << "  " << p->toString() << std::endl;
        }
    }
}