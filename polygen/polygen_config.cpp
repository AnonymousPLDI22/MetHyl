//
// Created by pro on 2021/11/16.
//

#include "polygen_config.h"
#include <unordered_set>
#include <algorithm>

using namespace polygen;

namespace {
    const std::unordered_set<std::string> useful_operator = {"*", "sqr", "-", "pow"};
    void extractUsedComponent(Program* p, std::unordered_set<std::string>& operators, std::unordered_set<int>& int_consts) {
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp) {
            if (useful_operator.find(sp->semantics->name) != useful_operator.end()) operators.insert(sp->semantics->name);
            auto* cs = dynamic_cast<ConstSemantics*>(sp->semantics);
            if (cs && type::equal(cs->oup_type, TINT)) int_consts.insert(cs->value.getInt());
        }
        for (auto* sub: p->getSubPrograms()) {
            extractUsedComponent(sub, operators, int_consts);
        }
    }
}

void PolyGenConfig::insertComponent(Program *p) {
    std::unordered_set<std::string> operators;
    std::unordered_set<int> ints;
    // std::cout << "extract from " << p->toString() << std::endl;
    extractUsedComponent(p, operators, ints);
    for (auto& s: operators) extra_list.push_back(s);
    for (auto& w: ints) int_consts.push_back(w);
    std::sort(extra_list.begin(), extra_list.end());
    extra_list.resize(std::unique(extra_list.begin(), extra_list.end()) - extra_list.begin());
    std::sort(int_consts.begin(), int_consts.end());
    int_consts.resize(std::unique(int_consts.begin(), int_consts.end()) - int_consts.begin());
}

void PolyGenConfig::clear() {
    int_consts.clear(); int_consts.push_back(0); int_consts.push_back(1);
    extra_list.clear();
}