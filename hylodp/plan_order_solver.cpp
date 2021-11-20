//
// Created by pro on 2021/9/15.
//

#include "plan_order_solver.h"
#include "config.h"
#include "solver/grammar.h"
#include "solver/enumerator.h"
#include "glog/logging.h"
#include <iostream>
#include <cassert>
#include <algorithm>

PlanOrderSolver::PlanOrderSolver(InputSampler *_s, int timeout): sampler(_s) {
    task = sampler->t;
    if (timeout > 0) guard = new TimeGuard(timeout);
}

Grammar * PlanOrderSolver::buildDefaultGrammar(const TypeList &var_list, Type *oup_type) {
    Grammar* grammar = grammar::buildDefaultGrammar(var_list, oup_type);
    auto* cmp_symbol = new NonTerminal("cmp", TBOOL);
    for (auto cmp_type: {GEQ, LEQ, EQ}) {
        cmp_symbol->rule_list.push_back(new CmpRule(cmp_type, grammar->start_symbol));
    }
    auto symbol_list = grammar->symbol_list;
    symbol_list.push_back(cmp_symbol);
    return new Grammar(cmp_symbol, symbol_list);
}

int PlanOrderSolver::evaluateRange(Program *p) {
    auto *cp = dynamic_cast<CmpProgram *>(p);
    auto *key = cp ? cp->key : p;
    auto feature = key->toString();
    if (component_range_map.count(feature)) return component_range_map[feature];
    int mi = config::KINF, ma = -config::KINF;
    for (auto *log: log_list) {
        auto *oup = log->getOutputs(key);
        for (auto &w: *oup) {
            mi = std::min(mi, w.getInt());
            ma = std::max(ma, w.getInt());
        }
    }
    return component_range_map[feature] = ma - mi + 2 - (mi == 0 || ma == 0);
}

ProgramList PlanOrderSolver::getComponentList(int enumerate_num) {
    TypeList var_list = {task->plan_type};
    for (auto& t: task->env_list) var_list.push_back(t.type);
    auto* g = buildDefaultGrammar(var_list, TINT);
    auto* o = new DefaultOptimizer();
    auto* v = new AllCollectVerifier();
    EnumConfig ec(v, o, config::KINF, config::KINF);
    ec.calc_num = enumerate_num;
    return enumerate::synthesis(g, ec);
}

std::pair<int, int> PlanOrderSolver::getCost(CmpPreOrder *c) {
    int ma = -1, cost = 1;
    for (auto* p: c->cmp_list) {
        int range = evaluateRange(p);
        if (p->isEq() || range <= ma) cost *= range;
        else {
            ma = range; cost *= ma;
        }
    }
    return {cost, ma};
}

namespace {
    typedef std::pair<int, std::pair<int, int>> MergedExampleInfo;

    struct CandidateInfo {
        CmpProgram *program;
        int cost;
        std::vector<int> feature;
        CandidateInfo(CmpProgram* _p, int _cost): program(_p), cost(_cost), feature({cost, int(program->isEq()), program->size()}) {
        }
    };
}

std::vector<int> PlanOrderSolver::filterSeed(const std::vector<int> &id_list, CmpProgram *cmp) {
    std::vector<int> res;
    for (int x: id_list) {
        auto& info = seed_info_list[x];
        if (!log_list[info.pos]->buildExample(info.example).isValid(cmp)) res.push_back(x);
    }
    std::random_shuffle(res.begin(), res.end());
    return res;
}

ProgramList PlanOrderSolver::getCandidateAccordingToSeeds(const ProgramList &program_list, int num, const std::vector<int> &remain_seeds) {
    if (remain_seeds.size() < config::KC * num) return program_list;
    std::vector<int> merged_id;
    for (int i = 0; i < config::KC * num; ++i) {
        for (int pid: seed_info_list[remain_seeds[i]].valid_programs) {
            merged_id.push_back(pid);
        }
    }
    std::sort(merged_id.begin(), merged_id.end());
    merged_id.resize(std::unique(merged_id.begin(), merged_id.end()) - merged_id.begin());
    ProgramList res;
    for (int id: merged_id) res.push_back(program_list[id]);
    return res;
}

void PlanOrderSolver::prepareSeedInfo(const ProgramList &component_list, const std::vector<std::pair<int, std::pair<int, int> > > &example_space) {
    for (int i = 0; i < example_space.size() && i < config::KSeedExampleNum; ++i) {
        auto merged_example = example_space[i];
        auto example = log_list[merged_example.first]->buildExample(merged_example.second);
        std::vector<int> prog_id;
        for (int j = 0; j < component_list.size(); ++j) {
            if (example.isValid(component_list[j])) prog_id.push_back(j);
        }
        seed_info_list.push_back({merged_example.first, merged_example.second, prog_id});
    }
}

std::string PlanOrderSolver::getFeature(CmpProgram *p) {
    std::string res;
    for (auto* log: log_list) res += log->getFeature(p);
    return res;
}

CmpPreOrder* PlanOrderSolver::search(int num, const std::vector<Program*>& program_list, CmpPreOrder* order, std::vector<int> seeds, int current_ans) {
    std::vector<MergedExampleInfo> example_space;
    std::vector<std::vector<std::pair<int, int>>> separate_example_space(log_list.size());
    int tot = 0, limit = num ? config::KMaxExamplePerSearch : 1;
    for (int i = int(log_list.size()) - 1; i >= 0; --i) {
        int target_num = std::min(config::KMaxExamplePerExecution, limit - tot);
        auto res = log_list[i]->extractLocalExample(order, target_num);
        separate_example_space[i] = res;
        for (auto& local_example: res) example_space.emplace_back(i, local_example);
        tot += res.size();
    }
    std::random_shuffle(example_space.begin(), example_space.end());
    if (guard && guard->isTimeOut()) throw TimeOutError();
    if (tot == 0) return order;
    if (!num) {
        return nullptr;
    }

    if (num > 1 && seed_info_list.empty()) {
        prepareSeedInfo(program_list, example_space);
        for (int i = 0; i < seed_info_list.size(); ++i) seeds.push_back(i);
    }

    auto is_candidate = [=](CmpProgram* p) {
        assert(p);
        if (num == 1) {
            for (auto& full_example: example_space) {
                if (!log_list[full_example.first]->buildExample(full_example.second).isValid(p)) {
                    return false;
                }
            }
            return true;
        }
        int test_num = config::KC * num;
        bool is_all_false = true;
        for (int i = 0; i < test_num; ++i) {
            auto full_example = example_space[rand() % tot];
            if (log_list[full_example.first]->buildExample(full_example.second).isValid(p)) {
                is_all_false = false; break;
            }
        }
        if (is_all_false) return false;
        int satisfied_num = 0;
        try {
            for (int i = 0; i < log_list.size(); ++i) {
                satisfied_num += log_list[i]->getCoveredNum(p, separate_example_space[i]);
            }
        } catch (SemanticsError& e) {
            return false;
        }
        return satisfied_num * num >= tot;
    };

    ProgramList candidate_list;
    for (auto* p: getCandidateAccordingToSeeds(program_list, num, seeds)) {
        if (is_candidate(dynamic_cast<CmpProgram *>(p))) {
            candidate_list.push_back(p);
        }
    }
    std::pair<int, int> oc = getCost(order);
    std::vector<CandidateInfo> info_list;
    for (auto* p: candidate_list) {
        int range;
        try {
            range = evaluateRange(p);
        } catch (SemanticsError& e) {
            continue;
        }
        auto* cp = dynamic_cast<CmpProgram*>(p);
        int cost = oc.first;
        if (cp->isEq() || range <= oc.second) cost *= range; else cost *= oc.first;
        info_list.emplace_back(cp, cost);
    }
    if (info_list.empty()) return nullptr;
    std::sort(info_list.begin(), info_list.end(), [](const CandidateInfo& x, const CandidateInfo& y) {
        return x.feature < y.feature;
    });
    std::unordered_set<std::string> visited_list;
    int vis_num = 0;
    CmpPreOrder* ans = nullptr;
    for (auto& next: info_list) {
        auto feature = getFeature(next.program);
        if (visited_list.find(feature) != visited_list.end()) continue;
        if (next.cost >= current_ans) break;
        visited_list.insert(feature);
        vis_num++;
        auto* res = search(num - 1, program_list, order->insert(next.program), filterSeed(seeds, next.program), current_ans);
        if (res) {
            int now_cost = getCost(res).first;
            if (now_cost < current_ans){
                current_ans = now_cost; ans = res;
            }
        }
    }
    return ans;
}

CmpPreOrder * PlanOrderSolver::synthesis(int component_num, int enumerate_num) {
    seed_info_list.clear();
    auto component_list = getComponentList(enumerate_num);
    auto* cmp_eval = new CmpProgram(LEQ, task->eval);
    auto* cmp_order = new CmpPreOrder({cmp_eval});
    //for (int i = 0; i < 1000; ++i) std::cout << i << " " << component_list[i]->toString() << std::endl;
    //int kk; std::cin >> kk;
    /*auto* li = new Type(T_LIST, {TINT});
    auto* x = program::buildParam(0, new Type(T_PROD, {li, li}));
    auto* comp = new AccessProgram(x, 2);
    auto* size = new SemanticsProgram(semantics::string2Semantics("size"), {comp});
    auto* candidate = new CmpPreOrder({cmp_eval, new CmpProgram(EQ, size)});
    config::is_print = true;
    for (auto* log: log_list) {
        auto res = log->extractLocalExample(candidate, 10);
    }
    config::is_print = false;*/

    return search(component_num, component_list, cmp_order, {}, config::KINF);
}

PlanExecutionLog * PlanOrderSolver::verifyOrder(CmpPreOrder *order) {
    int total_size = 1;
    for (auto* p: order->cmp_list) total_size += p->size();
    total_size *= config::KRelationSolverTestNum;
    int plan_num = 0;
    //std::cout << "verify " << order->toString() << std::endl;
    for (int i = 0; plan_num <= total_size; ++i) {
        auto* log = sampler->getLog(config::KSampleVerifierTimeOut);
        //std::cout << log->start->state.toString() << " " << data::dataList2String(log->env) << std::endl;
        auto* s_log = new PlanExecutionLog(log);
        plan_num += s_log->plan_list.size();
        delete log;
        if (!s_log->verify(order)) {
            return s_log;
        }
        delete s_log;
    }
    return nullptr;
}

CmpPreOrder* PlanOrderSolver::synthesisPreOrder() {
    std::vector<int> size_list(config::KComponentUpperBound + 1);
    for (int i = 1; i <= config::KComponentUpperBound; ++i) {
        size_list[i] = config::KInitCmpNum / i;
    }
    while (1) {
        component_range_map.clear();
        if (guard && guard->isTimeOut()) throw TimeOutError();
        CmpPreOrder* res = nullptr;
        for (int i = 1; i <= config::KComponentUpperBound; ++i) {
            res = synthesis(i, size_list[i]);
            if (res) break;
        }
        if (res) {
            //LOG(INFO) << "Candidate " << res->toString();
            //if (res->cmp_list.size() == 2) config::is_print = true;
            auto* log = verifyOrder(res);
            if (log == nullptr) return res;
            log_list.push_back(log);
        } else {
            for (int i = 1; i <= config::KComponentUpperBound; ++i) {
                size_list[i] *= 2;
            }
        }
    }
}

PlanOrderSolver::~PlanOrderSolver() {
    for (auto* log: log_list) delete log;
}