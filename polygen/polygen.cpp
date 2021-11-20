//
// Created by pro on 2021/5/5.
//

#include "polygen.h"
#include "semantics.h"
#include "polygen_grammar.h"
#include "enumerator.h"
#include <algorithm>
#include <cassert>
#include "glog/logging.h"

using namespace polygen;

namespace {
    Program* mergeIte(std::vector<Program*>& term_list, std::vector<Program*>& condition_list) {
        int n = term_list.size();
        auto* program = term_list[n - 1];
        for (int i = n - 2; i >= 0; --i) {
            auto cond = condition_list[i]->toString();
            if (cond == "true") {
                program = term_list[i];
            } else if (cond == "false") {
                program = program;
            } else {
                program = new SemanticsProgram(semantics::string2Semantics("ite"),
                        {condition_list[i], term_list[i], program});
            }
        }
        return program;
    }

    PointExample* verify(Program* program, const std::vector<PointExample*>& example_space) {
        for (auto* example: example_space) {
            if (program->run(example->first) != example->second) {
                return example;
            }
        }
        return nullptr;
    }

    std::vector<PointExample*> verifyTerms(const std::vector<Program*>& term_list,
            const std::vector<PointExample*>& example_space, int num = 100) {
        std::vector<PointExample*> example_list;
        for (auto* example: example_space) {
            bool flag = false;
            try {
                for (auto *term: term_list) {
                    if (term->run(example->first) == example->second) {
                        flag = true;
                    }
                }
            } catch (SemanticsError& e) {
                flag = false;
            }
            if (!flag) {
                example_list.push_back(example);
                if (example_list.size() == num) return example_list;
            }
        }
        return example_list;
    }

    std::pair<std::vector<PointExample*>, std::vector<PointExample*> > verifyCondition(Program* condition,
            const std::vector<PointExample*>& positive_list, const std::vector<PointExample*>& negative_list,
            int num = 1) {
        std::vector<PointExample*> positive_result;
        for (auto* example: positive_list) {
            bool is_used = false;
            try {
                if (!condition->run(example->first).getBool()) is_used = true;
            } catch (SemanticsError& e) {
                is_used = true;
            }
            if (is_used) {
                positive_result.push_back(example);
                if (positive_result.size() == num) break;
            }
        }
        std::vector<PointExample*> negative_result;
        for (auto* example: negative_list) {
            bool is_used = false;
            try {
                if (condition->run(example->first).getBool()) is_used = true;
            } catch (SemanticsError &e) {
                is_used = true;
            }
            if (is_used) {
                negative_result.push_back(example);
                if (negative_result.size() == num) break;
            }
        }
        return std::make_pair(std::move(positive_result), std::move(negative_result));
    }

    std::vector<Program*> reorderTermList(const std::vector<Program*>& term_list,
            const std::vector<PointExample*>& example_list) {
        std::vector<std::pair<int, Program*>> count_list;
        for (auto* term: term_list) {
            int c = 0;
            for (auto* example: example_list) {
                if (term->run(example->first) == example->second) {
                    c += 1;
                }
            }
            count_list.emplace_back(c, term);
        }
        std::sort(count_list.begin(), count_list.end());
        std::vector<Program*> result;
        for (auto& info: count_list) {
            result.push_back(info.second);
        }
        return result;
    }
}

Program* PolyGen::synthesis(const std::vector<PointExample *> &full_example_list, TimeGuard* guard) {
    if (full_example_list.empty()) return program::buildConst(0);
    clearCache();
    /*LOG(INFO) << "overview polygen example space" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << example::pointExample2String(*(full_example_list[i])) << std::endl;
    }*/

    std::vector<Program*> term_list = {new SemanticsProgram(new ConstSemantics(0), {})};
    std::vector<PointExample*> counter_example_list;
    while (true) {
        if (guard && guard->isTimeOut()) throw TimeOutError();
        auto example_list = verifyTerms(term_list, full_example_list);
        if (example_list.empty()) break;
        for (auto* example: example_list) {
            counter_example_list.push_back(example);
        }
        term_list = term_solver->getTerms(counter_example_list, guard);
        if (term_list.empty()) return nullptr;
    }
    if (term_list.size() > 1) {
        LOG(INFO) << "term list" << std::endl;
        for (auto *t: term_list) std::cout << t->toString() << std::endl;
    }

    term_list = reorderTermList(term_list, full_example_list);
    std::vector<int> param_list; int param_num = full_example_list[0]->first.size();
    for (int i = 0; i < param_num; ++i) {
        param_list.push_back(i);
    }
    unifier->param_list = param_list;
    unifier->extra_list = term_list;

    std::vector<Program*> condition_list;
    std::vector<PointExample*> remain_list = full_example_list;
    for (int term_id = 0; term_id + 1 < term_list.size(); ++term_id) {
        auto* term = term_list[term_id];
        std::vector<PointExample*> full_positive_list, full_negative_list, free_list;
        for (auto* example: remain_list) {
            if (term->run(example->first) != example->second)
                full_negative_list.push_back(example);
            else {
                bool flag = false;
                for (int next_id = term_id + 1; next_id < term_list.size(); ++next_id) {
                    if (term_list[next_id]->run(example->first) == example->second) {
                        flag = true;
                        break;
                    }
                }
                if (!flag) full_positive_list.push_back(example);
                else free_list.push_back(example);
            }
        }

        std::vector<PointExample*> positive_list, negative_list;
        Program* condition = new SemanticsProgram(new ConstSemantics(new BoolValue(false)), {});
        while (1) {
            if (guard && guard->isTimeOut()) throw TimeOutError();
            auto counter_list = verifyCondition(condition, full_positive_list, full_negative_list);
            if (counter_list.first.empty() && counter_list.second.empty()) break;
            for (auto* example: counter_list.first) {
                positive_list.push_back(example);
            }
            for (auto* example: counter_list.second) {
                negative_list.push_back(example);
            }
            condition = unifier->getCondition(positive_list, negative_list, guard);
        }
        condition_list.push_back(condition);

        remain_list = full_negative_list;
        for (auto* example: free_list) {
            if (!condition->run(example->first).getBool()) {
                remain_list.push_back(example);
            }
        }
    }
    if (guard && guard->isTimeOut()) throw TimeOutError();
    return mergeIte(term_list, condition_list);
}

Program * PolyGen::refSynthesis(PointExampleSpace *example_space, TimeGuard *guard) {
    TypeList inp_types; //std::cout << "bf" << std::endl;
    for (auto& inp: example_space->getExample(0)->first) inp_types.push_back(inp.getType());
    auto* g = buildDefaultTermGrammar(inp_types, c, false);
    auto* optimizer = new OEOptimizer({}, config::KINF, guard);
    auto* verifier = new PointVerifier({});
    EnumConfig ec(verifier, optimizer);
    ec.num_limit = 1;
    Program* res;
    while (1) {
        auto res_list = enumerate::synthesis(g, ec);
        if (guard && guard->isTimeOut()) return nullptr;
        PointExample* counter = nullptr;
        res = res_list[0];
        int example_num = res->size() * config::KInitExampleNum;
        for (int i = 0; i < example_num; ++i) {
            auto* example = example_space->getExample(i);
            try {
                if (res->run(example->first) != example->second) {
                    counter = example;
                    break;
                }
            } catch (SemanticsError& e) {
                counter = example; break;
            }
        }
        if (!counter) break;
        verifier->example_list.push_back(counter);
        optimizer->test_inp_list.push_back(counter->first);
        optimizer->clear();
    }
    delete verifier; delete optimizer;
    return res;
}

Program * PolyGen::synthesis(PointExampleSpace *example_space, TimeGuard* guard) {
    if (c.isRef) return refSynthesis(example_space, guard);
    int num = c.KInitExampleNum * 10;
    std::vector<PointExample*> example_list;
    while (1) {
        for (int i = example_list.size(); i < num; ++i) {
            // std::cout << "get example " << i << std::endl;
            example_list.push_back(example_space->getExample(i));
        }
        auto* res = synthesis(example_list, guard);
        if (!res) return refSynthesis(example_space, guard);
        if (res->size() * c.KInitExampleNum > num) {
            num = std::max(num * 2, res->size() * c.KInitExampleNum);
        } else return res;
    }
}


PolyGen* polygen::buildDefaultPolyGen(const std::vector<int>& int_consts) {
    PolyGenConfig c; c.int_consts = int_consts;
    return new PolyGen(c);
}