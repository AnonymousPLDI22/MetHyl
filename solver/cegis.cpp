//
// Created by pro on 2021/9/16.
//

#include "cegis.h"

bool CEGISVerifier::isValid(Program *program) {
    for (auto* example: example_list) {
        if (!example->isValid(program)) return false;
    }
    return true;
}

CEGISVerifier::CEGISVerifier(const std::vector<CEGISExample *> &_example_list): example_list(_example_list) {}

Program * solver::trivialCEGIS(const std::vector<CEGISExample *> &example_list, Grammar *g, Optimizer* o) {
    std::vector<CEGISExample*> counter_example_list;
    if (!o) o = new DefaultOptimizer();
    while (1) {
        // synthesis
        auto* v = new CEGISVerifier(counter_example_list);
        o->clear();
        EnumConfig c(v, o);
        auto res_list = enumerate::synthesis(g, c);
        delete v;
        if (res_list.empty()) return nullptr;
        auto* res = res_list[0];
        bool is_new_counter = false;
        for (auto* example: example_list) {
            if (!example->isValid(res)) {
                counter_example_list.push_back(example);
                is_new_counter = true;
                break;
            }
        }
        if (!is_new_counter) return res;
    }
}