//
// Created by pro on 2021/10/20.
//

#include "state_mapping_solver.h"
#include "glog/logging.h"
#include <algorithm>
#include <cassert>

StateMappingSolver::StateMappingSolver(InputSampler *_sampler, int timeout): sampler(_sampler), task(_sampler->t), small_sampler(_sampler->getLimitedSampler()) {
    if (timeout > 0) guard = new TimeGuard(timeout);
}

int StateMappingSolver::evaluateRange(Program *p) {
    auto feature = p->toString();
    if (range_cache.count(feature)) return range_cache[feature];
    int mi = config::KINF, ma = -config::KINF;
    for (auto* log: log_list) {
        auto* oup = log->getOutputs(p);
        for (auto& w: *oup) {
            mi = std::min(mi, w.getInt());
            ma = std::max(ma, w.getInt());
        }
    }
    return range_cache[feature] = ma - mi + 1;
}

ProgramList StateMappingSolver::getComponentList(int enumerate_num) {
    TypeList var_list = type::unfoldProdType(task->state_type);
    for (auto& env_info: task->env_list) var_list.push_back(env_info.type);
    var_list.push_back(task->state_type);
    auto* g = grammar::buildDefaultGrammar(var_list, TINT);
    auto* o = new DefaultOptimizer(guard);
    auto* v = new AllCollectVerifier();
    EnumConfig ec(v, o, config::KINF, config::KINF);
    ec.calc_num = enumerate_num;
    auto res = enumerate::synthesis(g, ec);
    return res;
}

StateExecutionLog * StateMappingSolver::verifyRelation(KeyEqRelation *relation) {
    int total_size = 1;
    for (auto* key: relation->key_list) total_size += key->size();
    total_size *= config::KRelationSolverTestNum;
    for (auto* used_sampler: {sampler, small_sampler}) {
        int state_num = 0;
        for (; state_num <= total_size;) {
            auto *log = used_sampler->getLog(config::KSampleVerifierTimeOut);
            /*for (auto* state: log->state_list) {
                std::cout << "state " << state->state.toString() << " " << data::dataList2String(state->plan_list) << std::endl;
            }*/
            auto *s_log = new StateExecutionLog(log);
            state_num += s_log->state_list.size();
            delete log;
            try {
                if (!s_log->verify(relation)) {
                    return s_log;
                }
            } catch (SemanticsError& e) {
                return s_log;
            }
            delete s_log;
        }
    }
    return nullptr;
}

StateMappingSolver::~StateMappingSolver() {
    for (auto* log: log_list) delete log;
}

int StateMappingSolver::getCost(KeyEqRelation *relation) {
    int res = 1;
    for (auto* key: relation->key_list) {
        res *= evaluateRange(key);
    }
    return res;
}

namespace {
    typedef std::pair<int, int> LocalExample;
    typedef std::pair<int, LocalExample> FullExample;

    struct CandidateInfo {
        Program* p;
        int pos;
        int cost;
        std::pair<int, int> feature;
        CandidateInfo(Program* _p, int _pos, int _cost): p(_p), cost(_cost), pos(_pos), feature(cost, _pos) {
        }
    };
}

bool StateMappingSolver::runFullExample(const std::pair<int, std::pair<int, int> > &full_example, Program *p) {
    return log_list[full_example.first]->buildExample(full_example.second).isValid(p);
}

namespace {
    KeyEqRelation* insertRelation(KeyEqRelation* r, Program* p) {
        ProgramList key_list = r->key_list; key_list.push_back(p);
        return new KeyEqRelation(key_list);
    }
}

std::string StateMappingSolver::getFeature(Program *p) {
    std::string feature;
    for (auto* log: log_list) feature += "@" + log->getFeature(p);
    return feature;
}

void StateMappingSolver::prepareSeedInfo(const ProgramList &program_list, const std::vector<std::pair<int, std::pair<int, int> > > &example_space) {
    for (int _ = 0; _ < config::KSeedExampleNum; ++_) {
        std::unordered_set<int> valid_set;
        auto full_example = example_space[rand() % example_space.size()];
        for (int i = 0; i < program_list.size(); ++i) {
            if (runFullExample(full_example, program_list[i])) valid_set.insert(i);
        }
        seed_info_list.emplace_back(full_example.first, full_example.second, std::move(valid_set));
    }
}

std::vector<int> StateMappingSolver::getCandidateAccordingToSeeds(int num, std::vector<int> seeds) {
    int n = num * config::KC;
    if (seeds.size() < n) {
        std::vector<int> res;
        for (int x: full_set) res.push_back(x);
        return res;
    }
    std::random_shuffle(seeds.begin(), seeds.end());
    std::unordered_set<int> valid;
    for (int i = 0; i < n; ++i) {
        for (auto x: seed_info_list[seeds[i]].pos_set) valid.insert(x);
    }
    std::vector<int> res;
    for (auto x: valid) res.push_back(x);
    return res;
}

void StateMappingSolver::removeDuplicated(int pos) {
    for (auto& info: seed_info_list) {
        if (info.pos_set.find(pos) != info.pos_set.end()) {
            info.pos_set.erase(pos);
        }
    }
    if (full_set.find(pos) != full_set.end()) {
        full_set.erase(pos);
    }
}

std::vector<int> StateMappingSolver::getRemainSeeds(const std::vector<int> &seeds, int pos) {
    std::vector<int> res;
    for (int id: seeds) {
        if (seed_info_list[id].pos_set.find(pos) == seed_info_list[id].pos_set.end()) {
            res.push_back(id);
        }
    }
    return res;
}

KeyEqRelation * StateMappingSolver::search(int num, ProgramList &component_space, KeyEqRelation *relation, std::vector<int> seeds, int ans) {
    int limit_num = num == 0 ? 1 : config::KMaxExamplePerSearch;
    std::vector<FullExample> example_space;
    std::vector<std::vector<LocalExample>> separate_example_space(log_list.size());
    int tot = 0;
    for (int i = int(log_list.size()) - 1; i >= 0; --i) {
        int limit = std::min(config::KMaxExamplePerExecution, limit_num - tot);
        separate_example_space[i] = log_list[i]->extractExamples(relation, limit);
        tot += int(separate_example_space[i].size());
        for (auto& local_example: separate_example_space[i]) {
            example_space.emplace_back(i, local_example);
        }
    }
    if (guard && guard->isTimeOut()) throw TimeOutError();
    if (tot == 0) {
        std::cout << "find " << relation->toString() << std::endl;
        return relation;
    }
    if (!num) return nullptr;

    if (num > 1 && seed_info_list.empty()) {
        prepareSeedInfo(component_space, example_space);
        seeds.clear();
        for (int i = 0; i < seed_info_list.size(); ++i) seeds.push_back(i);
    }

    auto is_valid = [=](Program* p) {
        if (num == 1) {
            for (auto& full_example: example_space) {
                if (!runFullExample(full_example, p)) {
                    return false;
                }
            }
            return true;
        }
        int test_num = config::KC * num;
        bool is_all_false = true;
        for (int _ = 0; _ < test_num; ++_) {
            auto full_example = example_space[rand() % example_space.size()];
            if (runFullExample(full_example, p)) {
                is_all_false = false; break;
            }
        }
        if (is_all_false) return false;
        int covered_num = 0;
        for (int i = 0; i < log_list.size(); ++i) {
            covered_num += log_list[i]->getCoveredNum(separate_example_space[i], p);
        }
        return covered_num * num >= tot;
    };

    std::vector<CandidateInfo> info_list;
    for (auto pos: getCandidateAccordingToSeeds(num, seeds)) {
        try {
            if (is_valid(component_space[pos])) {
                info_list.emplace_back(component_space[pos], pos, evaluateRange(component_space[pos]));
            }
        } catch (SemanticsError& e) {
            removeDuplicated(pos);
        }
    }
    /*std::cout << relation->toString() << " " << getCost(relation) << std::endl;
    for (int i = 0; i < 20 && i < info_list.size(); ++i) {
        std::cout << "  " << info_list[i].p->toString() << " " << info_list[i].cost << std::endl;
    }*/
    std::sort(info_list.begin(), info_list.end(), [](const CandidateInfo& x, const CandidateInfo& y) {return x.feature < y.feature;});

    KeyEqRelation* res = nullptr;
    int current_cost = getCost(relation);
    std::unordered_set<std::string> visited_set;
    for (auto& info: info_list) {
        if (1ll * current_cost * info.cost >= ans) break;
        auto feature = getFeature(info.p);
        if (visited_set.find(feature) != visited_set.end()) {
            removeDuplicated(info.pos);
            continue;
        }
        visited_set.insert(feature);
        auto* sub_res = search(num - 1, component_space, insertRelation(relation, info.p), getRemainSeeds(seeds, info.pos), ans);
        if (sub_res) {
            res = sub_res; int new_cost = getCost(res);
            if (new_cost >= ans) {
                std::cout << sub_res->toString() << std::endl;
            }
            assert(new_cost < ans); ans = new_cost;
        }
    }
    return res;
}

KeyEqRelation * StateMappingSolver::synthesis(int num, int enumerate_num) {
    if (program_storage[num].empty()) {
        program_storage[num] = getComponentList(enumerate_num);
    }
    seed_info_list.clear();
    full_set.clear();
    for (int i = 0; i < program_storage[num].size(); ++i) {
        full_set.insert(i);
    }
    return search(num, program_storage[num], new KeyEqRelation({}), {}, config::KINF);
}

KeyEqRelation * StateMappingSolver::synthesisEqRelation() {
    std::vector<int> size_list(config::KComponentUpperBound + 1);
    program_storage.resize(config::KComponentUpperBound + 1);
    for (int i = 1; i <= config::KComponentUpperBound; ++i) {
        size_list[i] = config::KInitComponentTimeOut / i;
    }
    while (1) {
        range_cache.clear();
        KeyEqRelation* res = nullptr;
        for (int i = 1; i <= config::KComponentUpperBound; ++i) {
            res = synthesis(i, size_list[i]);
            if (res) break;
        }
        if (res) {
            LOG(INFO) << "Candidate relation " << res->toString();
            auto* log = verifyRelation(res);
            if (!log) return res;
            log_list.push_back(log);
        } else {
            LOG(INFO) << "Fail" << std::endl;
            for (int i = 1; i <= config::KComponentUpperBound; ++i) {
                size_list[i] *= 2;
                program_storage[i].clear();
            }
        }
    }
}
