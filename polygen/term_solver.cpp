//
// Created by pro on 2021/2/23.
//

#include "term_solver.h"
#include "bitset.h"
#include "config.h"
#include "enumerator.h"
#include "ilp_solver.h"
#include "polygen_grammar.h"
#include "glog/logging.h"

#include <set>
#include <random>
#include <cassert>

using namespace polygen;

namespace {
    PolyGenConfig local_c;

    int getConstBound() {
        int num = local_c.KTermIntMax;
        for (auto &int_const: local_c.int_consts) {
            num = std::max(num, std::abs(int_const));
        }
        return num;
    }

    int getOptimalAssignment(const std::vector<PointExample *> &example_list, std::vector<int> &result) {
        ILPConfig c;
        c.KConstMax = getConstBound();
        c.KCoefficientMax = local_c.KTermIntMax;
        return ilp::ILPFromExamples(example_list, result, c);
    }


    std::mt19937 rng;
    std::uniform_real_distribution<double> distribution(0, 1);

    Program *buildAssignment(const std::vector<int> &assignment) {
        std::vector<Program *> term_list;
        auto *times = semantics::string2Semantics("*");
        auto *plus = semantics::string2Semantics("+");
        int n = assignment.size() - 1;
        for (int i = 0; i < n; ++i) {
            if (assignment[i]) {
                Program *var = new SemanticsProgram(new ParamSemantics(i, TINT), {});
                if (assignment[i] != 1) {
                    Program *k = new SemanticsProgram(new ConstSemantics(assignment[i]), {});
                    term_list.push_back(new SemanticsProgram(times, {var, k}));
                } else {
                    term_list.push_back(var);
                }
            }
        }
        if (assignment[n] || term_list.empty()) {
            term_list.push_back(new SemanticsProgram(new ConstSemantics(assignment[n]), {}));
        }
        auto *result = term_list[0];
        for (int i = 1; i < term_list.size(); ++i) {
            result = new SemanticsProgram(plus, {result, term_list[i]});
        }
        return result;
    }

    bool is_print = false;

    bool verifyAssignment(const std::vector<int> &assignment, PointExample *example) {
        int oup = assignment[example->first.size()];
        for (int i = 0; i < example->first.size(); ++i) {
            oup += assignment[i] * example->first[i].getInt();
        }
        return oup == example->second.getInt();
    }

    struct AssignmentInfo {
        std::vector<int> assignment;
        Bitset P;
        int cost;

        AssignmentInfo(const std::vector<int> &_assignment) : assignment(_assignment), cost(0) {
            for (int &v: assignment) cost += !!v;
        }

        void update(const std::vector<PointExample *> &example_list) {
            for (int i = P.size(); i < example_list.size(); ++i) {
                P.append(verifyAssignment(assignment, example_list[i]));
            }
        }
    };

    struct AssignmentCmp {
        Bitset all;

        AssignmentCmp(const Bitset &_all) : all(_all) {}

        int operator()(AssignmentInfo *info_1, AssignmentInfo *info_2) {
            return (info_1->P & all).count() > (info_2->P & all).count();
        }
    };

    std::map<std::vector<int>, AssignmentInfo *> info_map, solved_sample;

    AssignmentInfo *
    buildAssignmentInfo(const std::vector<int> &assignment, const std::vector<PointExample *> &example_list) {
        if (info_map.count(assignment)) {
            auto *result = info_map[assignment];
            result->update(example_list);
            return result;
        }
        auto *info = new AssignmentInfo(assignment);
        info->update(example_list);
        info_map[assignment] = info;
        return info;
    }

    struct SampleInfo {
        int status;
        std::vector<int> example_list;
        AssignmentInfo *result;

        SampleInfo(const std::vector<int> &_example_list) : example_list(_example_list), status(0), result(nullptr) {
            std::sort(example_list.begin(), example_list.end());
        }
    };

    struct TermPlan {
        int N;
        std::vector<AssignmentInfo *> term_list;
        std::vector<int> rem_example;
        Bitset info;
        std::vector<SampleInfo *> sample_list;

        TermPlan(int _N, const std::vector<AssignmentInfo *> &_term_list, const Bitset &_info,
                 const std::vector<SampleInfo *> &_sample_list) :
                N(_N), term_list(_term_list), info(_info), sample_list(_sample_list) {}

        TermPlan(int _N, const std::vector<AssignmentInfo *> &_term_list) : N(_N), term_list(_term_list) {}

        bool checkCover(SampleInfo *sample) {
            for (int &id: sample->example_list) if (!info[id]) return false;
            return true;
        }

        void print() {
            std::cout << "term plan with n = " << N << " rem_num = " << rem_example.size() << std::endl;
            for (auto *info: term_list) {
                std::cout << buildAssignment(info->assignment)->toString() << std::endl;
            }
        }

        SampleInfo *generateSample(int type, int mid) {
            std::vector<int> id_list(N);
            if (type == 0) {
                for (int i = 0; i < N; ++i) id_list[i] = rem_example[std::rand() % mid];
            } else {
                while (1) {
                    bool is_valid = false;
                    for (int i = 0; i < N; ++i) {
                        int now = rem_example[std::rand() % rem_example.size()];
                        if (now >= mid) is_valid = true;
                        id_list[i] = now;
                    }
                    if (is_valid) break;
                }
            }
            // TODO: reduce duplicate sample
            return new SampleInfo(id_list);
        }
    };

    struct TermPlanCmp {
        bool operator()(TermPlan *plan_1, TermPlan *plan_2) {
            if (plan_1->N < plan_2->N) return true;
            if (plan_1->N > plan_2->N) return false;
            if (plan_1->term_list.size() < plan_2->term_list.size()) return true;
            if (plan_1->term_list.size() > plan_2->term_list.size()) return false;
            for (int i = 0; i < plan_1->term_list.size(); ++i) {
                if (plan_1->term_list[i]->assignment == plan_2->term_list[i]->assignment) continue;
                return plan_1->term_list[i]->assignment < plan_2->term_list[i]->assignment;
            }
            return false;
        }
    };

    std::set<TermPlan *, TermPlanCmp> plan_set;

    std::vector<PointExample *>
    idToExample(const std::vector<int> &id_list, const std::vector<PointExample *> &example_space) {
        std::vector<PointExample *> result;
        for (auto &id: id_list) result.push_back(example_space[id]);
        return result;
    }


    void performSample(SampleInfo *sample, const std::vector<PointExample *> &example_space) {
        if (sample->status) return;
        if (solved_sample.count(sample->example_list)) {
            sample->result = solved_sample[sample->example_list];
            if (sample->result) sample->result->update(example_space);
            return;
        }
        sample->status = 1;
        auto example_list = idToExample(sample->example_list, example_space);
        std::vector<int> assignment;
        int status = -1;
        status = getOptimalAssignment(example_list, assignment);
        if (status == 0) {
            solved_sample[sample->example_list] = nullptr;
            return;
        }
        sample->result = buildAssignmentInfo(assignment, example_space);
        solved_sample[sample->example_list] = sample->result;
    }

    std::vector<AssignmentInfo *> insertTermList(TermPlan *plan, AssignmentInfo *info) {
        auto result = plan->term_list;
        int pos = 0;
        while (pos < plan->term_list.size() && plan->term_list[pos]->assignment > info->assignment) {
            ++pos;
        }
        result.push_back(nullptr);
        for (int i = result.size() - 1; i > pos; --i) result[i] = result[i - 1];
        result[pos] = info;
        return result;
    }

    int calculateRandomTime(int branch_num, int N) {
        int ti = local_c.KRandomC;
        for (int i = 1; i <= N; ++i) ti *= branch_num;
        return ti;
    }

    void extendStart(TermPlan *plan, const std::vector<PointExample *> &example_space) {
        int pre_size = plan->info.size();
        int n = plan->N;
        for (int i = pre_size; i < example_space.size(); ++i) {
            plan->info.append(true);
            plan->rem_example.push_back(i);
        }
        for (auto *sample: plan->sample_list) {
            if (sample->result) sample->result->update(example_space);
        }
        std::vector<SampleInfo *> new_sample;
        int max_rem = local_c.KMaxTermNum - plan->term_list.size();
        int max_sample_num = calculateRandomTime(max_rem, n);
        double p = std::pow(1.0 * pre_size / plan->rem_example.size(), n);
        int pos = 0;
        while (pos < plan->sample_list.size() && new_sample.size() < max_sample_num) {
            if (distribution(rng) <= p) {
                new_sample.push_back(plan->sample_list[pos++]);
            } else {
                new_sample.push_back(plan->generateSample(1, pre_size));
            }
        }
        plan->sample_list = new_sample;
    }

    void
    extendPlan(TermPlan *plan, AssignmentInfo *info, TermPlan *pre, const std::vector<PointExample *> &example_space) {
        int pre_size = plan->info.size();
        int pre_num = plan->rem_example.size();
        for (int i = pre_size; i < example_space.size(); ++i) {
            if (!info->P[i] && pre->info[i]) {
                plan->info.append(true);
                plan->rem_example.push_back(i);
            } else {
                plan->info.append(false);
            }
        }
        for (auto *sample: plan->sample_list) {
            if (sample->result) sample->result->update(example_space);
        }
        std::vector<SampleInfo *> reusable_result;
        std::vector<SampleInfo *> new_sample;
        int n = plan->N;
        for (auto *sample: pre->sample_list) {
            if (plan->checkCover(sample) && sample->example_list[n - 1] >= pre_size) {
                reusable_result.push_back(sample);
            }
        }
        double p = std::pow(1.0 * pre_num / plan->rem_example.size(), n);
        int l_pos = 0, r_pos = 0;
        int max_rem = local_c.KMaxTermNum - plan->term_list.size();
        int max_sample_num = calculateRandomTime(max_rem, n);
        while ((l_pos < plan->sample_list.size() || r_pos < reusable_result.size()) &&
               new_sample.size() < max_sample_num) {
            if (distribution(rng) <= p) {
                if (l_pos < plan->sample_list.size()) {
                    new_sample.push_back(plan->sample_list[l_pos++]);
                } else {
                    new_sample.push_back(plan->generateSample(0, pre_num));
                }
            } else {
                if (r_pos < reusable_result.size()) {
                    new_sample.push_back(reusable_result[r_pos++]);
                } else {
                    new_sample.push_back(plan->generateSample(1, pre_num));
                }
            }
        }
        plan->sample_list = new_sample;
    }

    TermPlan *buildTermPlan(TermPlan *plan, AssignmentInfo *info, const std::vector<PointExample *> &example_space) {
        auto term_list = insertTermList(plan, info);
        auto *dummy_plan = new TermPlan(plan->N, term_list);
        auto it = plan_set.find(dummy_plan);
        if (it != plan_set.end()) {
            auto *result = *it;
            delete dummy_plan;
            extendPlan(result, info, plan, example_space);
            return result;
        } else {
            auto *result = dummy_plan;
            plan_set.insert(result);
            extendPlan(result, info, plan, example_space);
            return result;
        }
    }

    std::vector<AssignmentInfo *>
    getNextAssignment(TermPlan *plan, const std::vector<PointExample *> &example_space, int N, int rem_branch) {
        int ti = calculateRandomTime(rem_branch, N);
        int limit = (int(plan->rem_example.size()) - 1) / rem_branch + 1;
        while (plan->sample_list.size() < ti) {
            plan->sample_list.push_back(plan->generateSample(0, plan->rem_example.size()));
        }
        std::set<AssignmentInfo *> info_set;
        std::set<AssignmentInfo *> fail_set;
        std::vector<AssignmentInfo *> result;
        for (int i = 0; i < ti; ++i) {
            auto *sample = plan->sample_list[i];
            performSample(sample, example_space);
            auto *info = sample->result;
            if (!info) continue;
            if (info_set.find(info) == info_set.end() && (plan->info & info->P).count() >= limit) {
                info_set.insert(info);
                result.push_back(info);
            } else {
                fail_set.insert(info);
            }
        }
        return result;
    }

    std::set<TermPlan *> visited_plan;

    bool search(TermPlan *plan, const std::vector<PointExample *> &example_space, std::vector<Program *> &result, int N,
                int rem_branch, TimeGuard *guard) {
        if (rem_branch == 0 || plan->rem_example.empty()) {
            assert(plan->rem_example.empty());
            result.clear();
            for (auto *assignment: plan->term_list) {
                result.push_back(buildAssignment(assignment->assignment));
            }
            return true;
        }
        if (guard && guard->isTimeOut()) throw TimeOutError();
        if (is_print) plan->print();
        auto next_assignment = getNextAssignment(plan, example_space, N, rem_branch);
        std::sort(next_assignment.begin(), next_assignment.end(), AssignmentCmp(plan->info));
        for (auto *info: next_assignment) {
            auto *next_plan = buildTermPlan(plan, info, example_space);
            if (visited_plan.find(next_plan) == visited_plan.end()) {
                visited_plan.insert(next_plan);
                if (search(next_plan, example_space, result, N, rem_branch - 1, guard)) return true;
            }
        }
        return false;
    }

    bool imNonOptimal(Program *p) {
        auto *sp = dynamic_cast<SemanticsProgram *>(p);
        if (sp) {
            if (sp->semantics->name == "+" || sp->semantics->name == "neg" || sp->semantics->name == "-") return true;
            if (dynamic_cast<ConstSemantics *>(sp->semantics)) return true;
            if (sp->semantics->name == "*") {
                for (auto *sub: p->getSubPrograms()) {
                    if (imNonOptimal(sub)) return true;
                }
            }
        }
        return false;

    }

    std::vector<Program *>
    removeDuplicatedTerm(const std::vector<Program *> &term_list, const std::vector<PointExample *> &example_list) {
        std::set<std::string> S;
        std::vector<Program *> result;
        for (auto *term: term_list) {
            if (imNonOptimal(term)) continue;
            DataList oup_list;
            bool is_valid = true;
            for (auto *example: example_list) {
                try {
                    int res = term->run(example->first).getInt();
                    if (std::abs(res) > config::KILPInputMax && term->size() > 1) {
                        is_valid = false;
                        break;
                    }
                    oup_list.push_back(res);
                } catch (SemanticsError &e) {
                    is_valid = false;
                    break;
                }
            }
            if (!is_valid) continue;
            auto feature = data::dataList2String(oup_list);
            if (S.find(feature) == S.end()) {
                result.push_back(term);
                S.insert(feature);
            }
        }
        return result;
    }

    Program *rewriteTermWithAtoms(Program *term, const std::vector<Program *> &atom_list) {
        auto *sp = dynamic_cast<SemanticsProgram *>(term);
        if (sp) {
            auto *param_semantics = dynamic_cast<ParamSemantics *>(sp->semantics);
            if (param_semantics) {
                return atom_list[param_semantics->id]->copy();
            }
        }
        std::vector<Program *> sub_list = term->getSubPrograms();
        for (int i = 0; i < sub_list.size(); ++i) {
            sub_list[i] = rewriteTermWithAtoms(sub_list[i], atom_list);
        }
        return term->clone(sub_list);
    }
}

std::vector<Program *> TermSolver:: getTerms(const std::vector<PointExample *> &example_space, int N, int K, TimeGuard* guard) {
    visited_plan.clear();
    std::vector<int> full_example;

    auto* start = new TermPlan(N, {});
    auto it = plan_set.find(start);
    if (it == plan_set.end()) {
        plan_set.insert(start);
    } else {
        delete start; start = *it;
    }
    extendStart(start, example_space);
    std::vector<Program*> result;
    search(start, example_space, result, N, K, guard);
    return result;
}

std::vector<PointExample *> TermSolver::buildExtendedExamples(const std::vector<Program*>& atom_list, const std::vector<PointExample *> &example_list) {
    std::vector<PointExample*> complete_example_list;
    for (auto* example: example_list) {
        DataList inp_list;
        for (auto* term: atom_list) inp_list.emplace_back(term->run(example->first));
        complete_example_list.push_back(new PointExample(inp_list, example->second));
    }
    return complete_example_list;
}

std::vector<Program *> TermSolver::getTerms(const std::vector<PointExample *> &example_list, TimeGuard* guard) {
    if (example_list.empty()) return {new SemanticsProgram(new ConstSemantics(0), {})};
    TypeList inp_types;
    for (auto& inp: example_list[0]->first) inp_types.push_back(inp.getType());
    local_c = c;
    DataStorage inp_storage;
    for (int i = 0; i < example_list.size(); ++i) {
        inp_storage.push_back(example_list[i]->first);
    }
    auto* optimizer = new OEOptimizer(std::move(inp_storage), config::KILPInputMax);
    auto* verifier = new AllCollectVerifier();
    auto* atom_grammar = polygen::buildDefaultTermGrammar(inp_types, c, true);
    EnumConfig ec(verifier, optimizer);
    ec.calc_num = c.KEnumeratorTimeout;
    bool is_time_out = false;
    int step = 0;
    //for (int i = 0; i < 10 && i < example_list.size(); ++i) std::cout << example::pointExample2String(*example_list[i]) << std::endl;
    // atom_grammar->print();
    std::vector<std::vector<Program*>> atom_cache;
    std::vector<std::vector<PointExample*>> atom_example_cache;
    std::vector<int> atom_progress;
    std::set<std::string> atom_feature_map;
    auto extend_cache = [&]() {
        for (int _ = 0; _ < 3 && !is_time_out; _++) {
            ++step;
            ec.num_limit = (1 << step - 1) * c.KInitTermAtomNum;
            std::vector<Program*> atom_list = enumerate::synthesis(atom_grammar, ec);
            if (atom_list.size() < ec.num_limit) is_time_out = true;
            atom_list = removeDuplicatedTerm(atom_list, example_list);
            if (atom_list.size() > c.KAtomMaxNum) {
                is_time_out = true; atom_list.resize(c.KAtomMaxNum);
            }
            std::string atom_feature;
            for (auto* atom: atom_list) {
                atom_feature += atom->toString() + "@";
            }
            //std::cout << atom_feature << std::endl;
            if (atom_feature_map.find(atom_feature) != atom_feature_map.end()) continue;
            auto extended_example = buildExtendedExamples(atom_list, example_list);
            /*for (int i = 0; i < 10 && i < extended_example.size(); ++i) {
                std::cout << "  " << example::pointExample2String(*extended_example[i]) << std::endl;
            }*/
            atom_feature_map.insert(atom_feature);
            atom_cache.push_back(atom_list);
            atom_progress.push_back(0);
            atom_example_cache.emplace_back(std::move(extended_example));
            return true;
        }
        return false;
    };

    std::set<std::vector<int>> calculated_set;

    while (1) {
        if (extend_cache()) {
            for (auto* a: atom_cache[atom_cache.size() - 1]) {
                bool is_valid = true;
                for (auto* e: example_list) {
                    if (a->run(e->first) != e->second) {
                        is_valid = false;
                        break;
                    }
                }
                if (is_valid) return {a};
            }
        }
        bool is_progress = false;
        for (int si = 0; si < atom_cache.size(); ++si) {
            if (!atom_cache.empty()) clearCache();
            // std::cout << atom_cache[si].size() << std::endl;
            atom_progress[si] += 1;
            int N_limit = std::min(local_c.KMaxExampleNum, atom_progress[si]);
            int K_limit = std::min(local_c.KMaxTermNum, std::max(1,atom_progress[si]/2));
            for (int N = 1; N <= N_limit; ++N) {
                for (int K = 1; K <= K_limit; ++K) {
                    if (guard && guard->isTimeOut()) throw TimeOutError();
                    if (calculated_set.find({si, N, K}) != calculated_set.end()) continue;
                    is_progress = true;
                    /*std::cout << "find " << si << " " << N << " " << K << std::endl;
                    for (int i = 0; i < 10 && i < atom_example_cache[si].size(); ++i) {
                        std::cout << example::pointExample2String(*atom_example_cache[si][i]) << std::endl;
                    }*/
                    calculated_set.insert({si, N, K});
                    auto result = getTerms(atom_example_cache[si], N, K, guard);
                    if (!result.empty()) {
                        for (auto &atom_example_list: atom_example_cache) {
                            for (auto *example: atom_example_list) {
                                delete example;
                            }
                        }
                        for (int i = 0; i < result.size(); ++i) {
                            result[i] = rewriteTermWithAtoms(result[i], atom_cache[si]);
                        }
                        return result;
                    }
                }
            }
        }
        if (!is_progress) {
            LOG(INFO) << "fail";
            return {};
        }
        // local_c.KMaxTermNum += 1;
        // local_c.KMaxTermNum
    }
}

void TermSolver::clearCache() {
    info_map.clear();
    solved_sample.clear();
    plan_set.clear();
    visited_plan.clear();
}