//
// Created by pro on 2021/5/5.
//

#include "sc_solver.h"
#include "polygen/polygen_grammar.h"
#include "enumerator.h"
#include <unordered_set>
#include <cassert>
#include "glog/logging.h"

using namespace autolifter;

namespace {
    void _flattenInput(const Data& w, DataList& res) {
        if (w.getType()->type == T_INT) {
            res.push_back(w);
            return;
        } else if (w.getType()->type == T_PROD) {
            for (auto& sub: w.getProdContents()) {
                _flattenInput(sub, res);
            }
            return;
        }
        assert(0);
    }

    DataList flattenInput(const DataList& inp) {
        DataList res;
        for (auto& w: inp) _flattenInput(w, res);
        return res;
    }

    bool isAllInt(Type* type) {
        if (type->type == T_INT) return true;
        if (type->type == T_PROD) {
            for (auto* sub: type->param) {
                if (!isAllInt(sub)) return false;
            }
            return true;
        }
        return false;
    }

    void buildAccessForAllComponent(Program* cur, ProgramList& res) {
        auto* type = cur->oup_type;
        if (type->type == T_INT) {
            res.push_back(cur->copy()); return;
        }
        if (type->type == T_PROD) {
            for (int i = 0; i < type->param.size(); ++i) {
                auto* now = new AccessProgram(cur, i + 1);
                buildAccessForAllComponent(now, res);
            }
            return;
        }
        assert(false);
    }
}

ScSolver::ScSolver(Task *_task, const std::vector<Program *> &_lifting_list, TimeGuard* _guard): task(_task), guard(_guard),
                                                                              lifting_list(_lifting_list), client_solver(new polygen::PolyGen(task->config.poly_config)) {
    acquireExample(task->config.KInitExampleNum);
}

void ScSolver::insertExample(PointExample &&example) {
    auto feature = data::dataList2String(example.first) + "->" + example.second.toString();
    if (example_set.find(feature) == example_set.end()) {
        example_set.insert(feature);
        io_example_space.push_back(new PointExample(example.first, example.second));
    }
}

void ScSolver::acquireExample(int limit) {
    while (example_pos < limit) {
        try {
            auto example = task->buildCExample(lifting_list, example_pos);
            if (!example.second.isNull()) insertExample(std::move(example));
        } catch (SemanticsError& e) {
        }
        example_pos += 1;
    }
}

Program* ScSolver::enumSynthesis(bool is_force) {
    TypeList inp_types; //std::cout << "bf" << std::endl;
    for (auto& inp: io_example_space[0]->first) inp_types.push_back(inp.getType());
    auto* g = polygen::buildDefaultTermGrammar(inp_types, task->config.poly_config, false);
    // g->print();
    auto* optimizer = new OEOptimizer({}, config::KINF, guard);
    auto* verifier = new PointVerifier({});
    EnumConfig ec(verifier, optimizer);
    ec.num_limit = 1;
    if (!program::isRef(task->p) && !is_force) ec.calc_num = config::KInitComponentTimeOut;
    Program* res;
    while (1) {
        auto res_list = enumerate::synthesis(g, ec);
        if (res_list.empty()) return nullptr;
        PointExample* counter = nullptr;
        res = res_list[0];
        // std::cout << "candidate " << res->toString() << std::endl;
        int example_num = res->size() * config::KInitExampleNum;
        if (program::isNil(task->cache->m)) example_num /= 10;
        acquireExample(example_num);
        for (auto* example: io_example_space) {
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

Program * ScSolver::synthesis() {
    //LOG(INFO) << "Value example space" << std::endl;
    acquireExample(100);
    if (io_example_space.empty()) return program::buildConst(0);
    //for (int i = 0; i < io_example_space.size(); ++i) std::cout << example::pointExample2String(*io_example_space[i]) << std::endl;
    auto* res = enumSynthesis();
    if (res) return res;
    while (1) {
        auto* program = client_solver->synthesis(io_example_space, guard);
        if (!program) return enumSynthesis(true);
        int pre_size = io_example_space.size();
        int needed_examples = program->size() * task->config.KInitExampleNum;
        if (program::isNil(task->cache->m)) needed_examples /= 10;
        acquireExample(needed_examples);
        bool is_valid = true;
        for (int i = pre_size; i < io_example_space.size(); ++i) {
            auto* example = io_example_space[i];
            try {
                if (program->run(example->first) != example->second) {
                    is_valid = false;
                    break;
                }
            } catch (SemanticsError& e) {
                is_valid = false; break;
            }
        }
        if (is_valid) {
            return program;
        }
    }
}