//
// Created by pro on 2021/10/20.
//

#include "ilp_solver.h"
#include "gurobi_c++.h"
#include <cassert>
#include <algorithm>

namespace {
    int convertToInt(double x) {
        int w = int(x);
        while (w < x - 0.5) ++w;
        while (w > x + 0.5) --w;
        return w;
    }

    GRBEnv env(true);
    bool is_initialized = false;

    void initializeEnv() {
        is_initialized = true;
        env.set("LogFile", "gurobi.log");
        env.start();
    }

    bool verify(const std::vector<int>& result, PointExample* example) {
        int res = result[result.size() - 1];
        for (int i = 0; i + 1 < result.size(); ++i) {
            res += result[i] * example->first[i].getInt();
        }
        return res == example->second.getInt();
    }
}

bool ilp::ILPFromExamples(const std::vector<PointExample *> &example_list, std::vector<int> &result, ILPConfig c) {
    if (!is_initialized) initializeEnv();
    int n = example_list[0]->first.size();

    GRBModel model = GRBModel(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    std::vector<GRBVar> var_list;
    std::vector<GRBVar> bound_list;
    for (int i = 0; i <= n; ++i) {
        std::string name_var = "var" + std::to_string(i);
        std::string name_bound = "bound" + std::to_string(i);
        int bound = i < n ? c.KCoefficientMax : c.KConstMax;
        var_list.push_back(model.addVar(-bound, bound, 0.0, GRB_INTEGER, name_var));
        if (i == n) {
            bound_list.push_back(model.addVar(0, 1, 0.0, GRB_INTEGER, name_bound));
            model.addConstr(var_list[i] <= bound * bound_list[i], "rbound" + std::to_string(i));
            model.addConstr(var_list[i] >= -bound * bound_list[i], "lbound" + std::to_string(i));
        } else {
            bound_list.push_back(model.addVar(0, bound, 0.0, GRB_INTEGER, name_bound));
            model.addConstr(var_list[i] <= bound_list[i], "rbound" + std::to_string(i));
            model.addConstr(var_list[i] >= -bound_list[i], "lbound" + std::to_string(i));
        }
    }

    int id = 0;
    for (auto& example: example_list) {
        GRBLinExpr expr = var_list[n];
        for (int i = 0; i < n; ++i) {
            expr += example->first[i].getInt() * var_list[i];
        }
        model.addConstr(expr == example->second.getInt(), "cons" + std::to_string(id++));
    }
    GRBLinExpr target = 0;
    for (auto bound_var: bound_list) {
        target += bound_var;
    }
    model.setObjective(target, GRB_MINIMIZE);
    model.optimize();
    int status = model.get(GRB_IntAttr_Status);
    if (status == GRB_INFEASIBLE) {
        result.clear();
        return false;
    }
    result.clear();
    for (auto var: var_list) {
        result.push_back(convertToInt(var.get(GRB_DoubleAttr_X)));
    }
    for (auto* example: example_list) {
        assert(verify(result, example));
    }
    return true;
}

bool ilp::ILPFromExampleCEGIS(std::vector<PointExample *> example_list, std::vector<int> &result, ILPConfig c) {
    std::random_shuffle(example_list.begin(), example_list.end());
    int init_num = std::max(1, std::min(int(example_list.size()), int(example_list[0]->first.size()) + 1));
    auto cegis_verify = [=](const std::vector<int>& res) -> PointExample* {
        for (auto* example: example_list) {
            if (!verify(result, example)) return example;
        }
        return nullptr;
    };
    std::vector<PointExample*> counter_example_list;
    for (int i = 0; i < init_num; ++i) {
        counter_example_list.push_back(example_list[i]);
    }
    while (true) {
        if (!ILPFromExamples(counter_example_list, result, c)) {
            result.clear();
            return false;
        }
        auto* counter_example = cegis_verify(result);
        if (counter_example) {
            counter_example_list.push_back(counter_example);
        } else return true;
    }
}