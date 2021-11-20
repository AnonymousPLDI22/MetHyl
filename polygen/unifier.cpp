//
// Created by pro on 2021/5/5.
//

#include "unifier.h"
#include "polygen_grammar.h"
#include "enumerator.h"
#include "config.h"
#include <set>
#include <cassert>
#include "glog/logging.h"

using namespace polygen;

namespace {
    Program* mergeClause(const std::vector<Program*>& cmp_list) {
        if (cmp_list.empty()) return new SemanticsProgram(new ConstSemantics(new BoolValue(true)), {});
        auto* clause = cmp_list[0]->copy();
        for (int i = 1; i < cmp_list.size(); ++i) {
            clause = new SemanticsProgram(semantics::string2Semantics("&&"), {clause, cmp_list[i]->copy()});
        }
        return clause;
    }

    Program* mergeCondition(const std::vector<Program*>& clause_list) {
        Program* result = nullptr;
        if (clause_list.empty()) {
            return new SemanticsProgram(new ConstSemantics(new BoolValue(false)), {});
        }
        for (auto* program: clause_list) {
            if (!result) result = program;
            else result = new SemanticsProgram(semantics::string2Semantics("||"), {result, program});
        }
        return result;
    }

    Program* rewriteWithExtra(Program* program, const std::vector<Program*>& extra_list) {
        auto* sp = dynamic_cast<SemanticsProgram*>(program);
        if (sp) {
            auto *param = dynamic_cast<ParamSemantics *>(sp->semantics);
            if (param) {
                int id = param->id;
                if (id < 0) return rewriteWithExtra(extra_list[-id - 1]->copy(), extra_list);
                return program->copy();
            }
        }
        std::vector<Program*> sub_list = program->getSubPrograms();
        for (int i = 0; i < sub_list.size(); ++i) {
            sub_list[i] = rewriteWithExtra(sub_list[i], extra_list);
        }
        if (sp && sp->semantics->name == "int") return sub_list[0];
        return program->clone(sub_list);
    }

    struct CmpPlan {
        std::vector<CmpInfo*> cmp_list;
        Bitset rem_example;
        CmpPlan(const std::vector<CmpInfo*>& _cmp_list, const Bitset& _rem): cmp_list(_cmp_list), rem_example(_rem) {}
        CmpPlan() = default;

        void print() const {
            std::cout << "rem " << rem_example.count() << std::endl;
            for (int i = 0; i < cmp_list.size(); ++i) {
                std::cout << cmp_list[i]->cmp->toString() << std::endl;
            }
        }
    };

    int operator < (const CmpPlan& cmp_1, const CmpPlan& cmp_2) {
        return cmp_1.cmp_list.size() < cmp_2.cmp_list.size() || (cmp_1.cmp_list.size() == cmp_2.cmp_list.size() && cmp_1.rem_example < cmp_2.rem_example);
    }

    struct ClausePlan {
        std::vector<int> cmp_id_list;
        Bitset P_info, N_info;
        ClausePlan(const std::vector<int>& _list, const Bitset& _P, const Bitset& _N): cmp_id_list(_list), P_info(_P), N_info(_N) {}
        bool cover(ClausePlan* plan) {
            return P_info.checkCover(plan->P_info) && plan->N_info.checkCover(N_info);
        }
    };

    int cmpCmpInfo(CmpInfo* info_1, CmpInfo* info_2) {
        return info_1->cmp->toString() < info_2->cmp->toString();
    }

    std::vector<CmpInfo*> insertNewCmp(const CmpPlan& plan, CmpInfo* info) {
        std::vector<CmpInfo*> cmp_list = plan.cmp_list;
        cmp_list.emplace_back();
        int pos = 0;
        while (pos + 1 < cmp_list.size() && cmpCmpInfo(cmp_list[pos], info)) ++pos;
        for (int i = int(cmp_list.size()) - 1; i > pos; --i) cmp_list[i] = cmp_list[i - 1];
        cmp_list[pos] = info;
        return cmp_list;
    }

    ClausePlan* mergePlan(ClausePlan* l, ClausePlan* r) {
        std::vector<int> id_list = l->cmp_id_list;
        for (auto id: r->cmp_id_list) id_list.push_back(id);
        return new ClausePlan(id_list, l->P_info & r->P_info, l->N_info & r->N_info);
    }


    std::vector<ClausePlan*> reduceClause(std::vector<ClausePlan*>&& plan_list) {
        std::vector<std::vector<int>> feature_list;
        for (int i = 0; i < plan_list.size(); ++i) {
            auto* clause = plan_list[i];
            feature_list.push_back({-clause->P_info.count(), clause->N_info.count(), -int(clause->cmp_id_list.size()), i});
        }
        std::sort(feature_list.begin(), feature_list.end());
        std::vector<ClausePlan*> result;
        for (auto& feature: feature_list) {
            auto* clause = plan_list[feature[3]];
            bool is_duplicated = false;
            for (auto* pre_plan: result) {
                if (pre_plan->cover(clause)) {
                    is_duplicated = true;
                    break;
                }
            }
            if (!is_duplicated) result.push_back(clause); else delete clause;
        }
        return result;
    }

    std::vector<ClausePlan*> divideAndConquerNextClause(int l, int r, const std::vector<CmpInfo*>& cmp_info, const Bitset& rem_example, int limit, TimeGuard* guard) {
        std::vector<ClausePlan*> result;
        if (l == r) {
            auto* info = cmp_info[l];
            result.push_back(new ClausePlan({}, rem_example, Bitset(cmp_info[0]->N.size(), true)));
            auto rem_P = info->P & rem_example;
            if (rem_P.count() >= limit) {
                result.push_back(new ClausePlan({l}, rem_P, info->N));
            }
            return reduceClause(std::move(result));
        }
        int mid = l + r >> 1;
        auto l_result = divideAndConquerNextClause(l, mid, cmp_info, rem_example, limit, guard);
        auto r_result = divideAndConquerNextClause(mid + 1, r, cmp_info, rem_example, limit, guard);
        for (auto* l_plan: l_result) {
            if (guard && guard->isTimeOut()) break;
            for (auto* r_plan: r_result) {
                if (l == 0 && r == cmp_info.size() && (l_plan->N_info & r_plan->P_info).count()) continue;
                auto rem_P = l_plan->P_info & r_plan->P_info;
                if (rem_P.count() >= limit) {
                    result.push_back(mergePlan(l_plan, r_plan));
                }
            }
        }
        for (auto* plan: l_result) delete plan;
        for (auto* plan: r_result) delete plan;
        result = reduceClause(std::move(result));
        return result;
    }

    bool searchForSimpleClause(int id, const std::vector<Bitset>& prefix, const Bitset& now_n,
                               const std::vector<CmpInfo*>& cmp_info, std::vector<CmpInfo*>& result) {
        if (id == -1) return true;
        // std::cout << now_n.size() << " " << prefix[id].size() << std::endl;
        if ((now_n & prefix[id]).count() == 0) {
            if (searchForSimpleClause(id - 1, prefix, now_n, cmp_info, result)) {
                return true;
            }
        }
        result.push_back(cmp_info[id]);
        if (searchForSimpleClause(id - 1, prefix, now_n & cmp_info[id]->N, cmp_info, result)) {
            return true;
        }
        result.pop_back();
        return false;
    }

    CmpInfo* simplifyPlanGreedily(ClausePlan* plan, const std::vector<CmpInfo*>& cmp_info) {
        std::vector<CmpInfo*> used_info_list, result_info_list;
        for (int id: plan->cmp_id_list) {
            used_info_list.push_back(cmp_info[id]);
        }
        int n_n = plan->N_info.size();
        int p_n = plan->P_info.size();
        Bitset rem_n(n_n, true);
        while (rem_n.count()) {
            CmpInfo* best = nullptr;
            double best_cover_num = -1;
            for (auto* info: used_info_list) {
                double num = (rem_n.count() - (info->N & rem_n).count());
                if (num > best_cover_num) {
                    best_cover_num = num;
                    best = info;
                }
            }
            assert(best);
            result_info_list.push_back(best);
            rem_n = rem_n & best->N;
        }
        Bitset now_p = Bitset(p_n, true);
        std::vector<Program*> result_program;
        for (auto* cmp: result_info_list) {
            now_p = now_p & cmp->P;
            result_program.push_back(cmp->cmp);
        }
        delete plan;
        return new CmpInfo(mergeClause(result_program), now_p, rem_n);
    }

    std::vector<CmpInfo*> reorder(const std::vector<CmpInfo*>& clause_list) {
        std::vector<std::pair<int, int>> info_list;
        for (int i = 0; i < clause_list.size(); ++i) {
            info_list.emplace_back(-clause_list[i]->P.count(), i);
        }
        std::sort(info_list.begin(), info_list.end());
        std::vector<std::pair<int, CmpInfo*>> result;
        for (auto& info_pair: info_list) {
            auto* info = clause_list[info_pair.second];
            bool is_duplicated = false;
            for (auto& pre_info: result) {
                if (pre_info.second->P.checkCover(info->P)) {
                    is_duplicated = true;
                }
            }
            if (!is_duplicated) result.emplace_back(info_pair.second, info);
        }
        std::sort(result.begin(), result.end());
        std::vector<CmpInfo*> res;
        for (auto& info_pair: result) {
            res.push_back(info_pair.second);
        }
        return res;
    }

    std::set<CmpPlan> visited_plan;

    Program* searchForCondition(const CmpPlan& plan, const std::vector<CmpInfo*>& cmp_info, int rem_or, TimeGuard* guard) {
        if (visited_plan.find(plan) != visited_plan.end()) return nullptr;
        if (guard && guard->isTimeOut()) throw TimeOutError();
        // plan.print();
        visited_plan.insert(plan);
        if (rem_or == 0 || plan.rem_example.count() == 0) {
            assert(plan.rem_example.count() == 0);
            std::vector<Program*> clause_list;
            for (auto* cmp_info: plan.cmp_list) {
                clause_list.push_back(cmp_info->cmp);
            }
            return mergeCondition(clause_list);
        }
        int rem_number = plan.rem_example.count();
        int limit = (rem_number - 1) / rem_or + 1;
        std::vector<CmpInfo*> clause_list;
        for (auto* clause_plan: divideAndConquerNextClause(0, cmp_info.size() - 1, cmp_info, plan.rem_example, limit, guard)) {
            if (clause_plan->N_info.count()) continue;
            auto* result = simplifyPlanGreedily(clause_plan, cmp_info);
            if (result) {
                clause_list.push_back(result);
            }
        }
        reorder(clause_list);
        for (auto* cmp: clause_list) {
            auto cmp_list = insertNewCmp(plan, cmp);
            auto* result = searchForCondition(CmpPlan(cmp_list, plan.rem_example & (~cmp->P)), cmp_info, rem_or - 1, guard);
            if (result) return result;
        }
        return nullptr;
    }

    Program* searchForCondition(const std::vector<CmpInfo*>& cmp_info, int K, TimeGuard* guard) {
        int p_n = cmp_info[0]->P.size();
        Bitset full_positive(p_n, true);
        CmpPlan empty_plan({}, full_positive);
        visited_plan.clear();
        return searchForCondition(empty_plan, cmp_info, K, guard);
    }

    std::string getFeature(const std::vector<CmpInfo*>& info_list) {
        std::vector<std::string> result;
        for (auto* info: info_list) {
            result.push_back(info->cmp->toString());
        }
        std::sort(result.begin(), result.end());
        std::string feature;
        for (auto& s: result) {
            feature += "@" + s;
        }
        return feature;
    }

    bool executeCmp(Program* cmp, const DataList& inp) {
        return cmp->run(inp).getBool();
    }
}

std::vector<CmpInfo *> Unifier::excludeDuplicated(const std::vector<Program *> &program_list) {
    std::vector<Program*> valid_program_list;
    std::vector<std::pair<Bitset, Bitset>> info_list;
    std::vector<CmpInfo*> result;
    std::vector<bool> is_duplicate;
    int now = 0;
    for (auto* program: program_list) {
        Bitset N_status, P_status;
        for (auto* example: P) P_status.append(executeCmp(program, example->first));
        for (auto* example: N) N_status.append(executeCmp(program, example->first));
        valid_program_list.push_back(program);
        info_list.emplace_back(P_status, N_status);
        is_duplicate.push_back(false);
    }
    for (int i = int(valid_program_list.size()) - 1; i >= 0; --i) {
        for (int j = 0; j < valid_program_list.size(); ++j) {
            if (j != i && !is_duplicate[j] && info_list[j].first.checkCover(info_list[i].first) && info_list[i].second.checkCover(info_list[j].second)) {
                is_duplicate[i] = true;
                break;
            }
        }
    }
    for (int i = 0; i < valid_program_list.size(); ++i) {
        if (!is_duplicate[i]) {
            result.push_back(new CmpInfo(valid_program_list[i], info_list[i].first, info_list[i].second));
        }
    }
    return result;
}

bool Unifier::verifySolvable(const std::vector<CmpInfo *> &cmp_list) {
    if (cmp_list.empty()) return false;
    int p_n = cmp_list[0]->P.size();
    int n_n = cmp_list[0]->N.size();
    for (int i = 0; i < p_n; ++i) {
        Bitset n_status(n_n, true);
        for (auto* cmp_info: cmp_list) {
            if (cmp_info->P[i]) {
                n_status = n_status & cmp_info->N;
            }
        }
        if (n_status.count()) {
            return false;
        }
    }
    return true;
}

Program * Unifier::getCondition(const std::vector<PointExample *> &positive_example, const std::vector<PointExample *> &negative_example, TimeGuard* guard) {
    if (positive_example.empty()) return program::buildConst(new BoolValue(false));
    if (negative_example.empty()) return program::buildConst(new BoolValue(true));
    P = positive_example; N = negative_example;

    DataStorage inp_storage;
    {
        for (auto* p: positive_example) inp_storage.push_back(p->first);
        for (auto* n: negative_example) inp_storage.push_back(n->first);
    }
    TypeList inp_types;
    for (auto& inp: positive_example[0]->first) inp_types.push_back(inp.getType());
    auto* optimizer = new OEOptimizer(std::move(inp_storage));
    auto* verifier = new AllCollectVerifier();
    auto* predicate_grammar = polygen::buildDefaultConditionGrammar(inp_types, c, extra_list);
    /*predicate_grammar->print();
    std::cout << "P examples" << std::endl;
    for (int i = 0; i < positive_example.size() && i < 10; ++i) std::cout << example::pointExample2String(*positive_example[i]) << std::endl;
    std::cout << "N examples" << std::endl;
    for (int i = 0; i < negative_example.size() && i < 10; ++i) std::cout << example::pointExample2String(*negative_example[i]) << std::endl;*/

    /*auto* x1 = program::buildParam(1, inp_types[1]);
    auto* x11 = new AccessProgram(x1, 1);
    auto* x12 = new AccessProgram(x1, 2);
    auto* x2 = program::buildParam(2, inp_types[2]);
    auto* x0 = program::buildParam(0, inp_types[0]);
    auto* x01 = new AccessProgram(x0, 1);
    auto* cmp1 = new SemanticsProgram(semantics::string2Semantics("<="), {x1})*/
    EnumConfig ec(verifier, optimizer);
    bool is_time_out = false;
    int step = 0;
    std::vector<std::vector<CmpInfo*>> info_lists;
    std::unordered_set<std::string> cmp_feature_set;
    auto extend_predicate_list = [&]() {
        int lim = 100; if (step > 0) lim = 3, ec.calc_num = c.KEnumeratorTimeout; else ec.calc_num = c.KEnumeratorTimeout * 10;
        for (int _ = 0; _ < lim && !is_time_out; _++) {
            ++step; ec.num_limit = step * c.KInitPredicateNum;
            std::vector<Program*> predicate_list = enumerate::synthesis(predicate_grammar, ec);
            if (predicate_list.size() < ec.num_limit) {

                is_time_out = true;
            }
            //std::cout << "asd " << std::endl;
            // for (int i = 0; i < predicate_list.size() && i < 100; ++i) std::cout << predicate_list[i]->toString() << std::endl;
            auto info_list = excludeDuplicated(predicate_list);
            // std::cout << "info list " << verifySolvable(info_list) << std::endl;
            // for (auto* info: info_list) std::cout << info->cmp->toString() << std::endl;
            if (!verifySolvable(info_list)) continue;
            //for (auto* info: info_list) std::cout << info->cmp->toString() << std::endl;
            //int kk; std::cin >> kk;
            auto feature = getFeature(info_list);
            if (cmp_feature_set.find(feature) != cmp_feature_set.end()) continue;
            cmp_feature_set.insert(feature);
            info_lists.push_back(info_list);
            break;
        }
    };

    int size_limit = 1, or_limit = 1;
    std::set<std::pair<int, int>> visited;

    for (int t_id = 0;; ++t_id) {
        for (int or_num = 1; or_num <= or_limit; ++or_num) {
            if (is_time_out && info_lists.empty()) throw TimeOutError();
            for (int size = 0; size < size_limit; ++size) {
                if (size == info_lists.size()) extend_predicate_list();
                if (size >= info_lists.size()) break;
                if (visited.find(std::make_pair(or_num, size)) != visited.end()) continue;
                visited.insert(std::make_pair(or_num, size));
                auto* condition = searchForCondition(info_lists[size], or_num, guard);
                if (guard && guard->isTimeOut()) throw TimeOutError();
                if (condition) {
                    // std::cout << condition->toString() << std::endl;
                    return condition;
                }
            }
        }
        or_limit = std::min(or_limit + 1, 3);
        size_limit += 1;
    }
    assert(0);
}