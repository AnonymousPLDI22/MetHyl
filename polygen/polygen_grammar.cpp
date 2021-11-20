//
// Created by pro on 2021/10/23.
//

#include "polygen_grammar.h"
#include "glog/logging.h"
#include <unordered_set>

using namespace polygen;
namespace {
    void collectSubTypes(Type* type, std::unordered_set<std::string>& type_set, TypeList& res) {
        type = type::getPure(type);
        auto feature = type->getName();
        if (type_set.find(feature) == type_set.end()) {
            res.push_back(type);
            type_set.insert(feature);
        }
        for (auto* sub: type->param) collectSubTypes(sub, type_set, res);
    }

    Grammar* buildIntGrammar(TypeList& inp_type, const PolyGenConfig& c, bool is_atom = false) {
        GrammarConfig gc;
        gc.op_list = {"+", "neg", "access", "lmove", "bmovel", "bmover", "baccess", "isleaf", "subtree"};
        if (is_atom) gc.op_list = {"+", "neg", "access", "baccess"};
        for (auto& name: c.extra_list) gc.op_list.push_back(name);
        for (auto& ic: c.int_consts) gc.const_list.emplace_back(ic);
        gc.var_list = inp_type;
        std::unordered_set<std::string> type_set;
        for (auto* type: inp_type) {
            collectSubTypes(type, type_set, gc.basic_types);
        }
        auto* g = grammar::buildGrammar(gc);
        return g;
    }
}

Grammar * polygen::buildDefaultTermGrammar(TypeList &inp_type, const PolyGenConfig &c, bool is_atom) {
    auto* ig = buildIntGrammar(inp_type, c, is_atom);
    auto* start = new NonTerminal("start", TINT);
    for (auto* rule: ig->start_symbol->rule_list) {
        auto* sr = dynamic_cast<SemanticsRule*>(rule);
        if (sr && is_atom) {
            if (dynamic_cast<ConstSemantics *>(sr->semantics)) continue;
            if (sr->semantics->name == "+" || sr->semantics->name == "neg" || sr->semantics->name == "-") continue;
        }
        start->rule_list.push_back(rule);
    }
    std::vector<NonTerminal*> symbol_list = {start};
    for (auto* symbol: ig->symbol_list) symbol_list.push_back(symbol);
    return new Grammar(start, symbol_list);
}

namespace {
    void insertAccess(Program* p, ProgramList& cared) {
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp && sp->semantics->name == "access") {
            cared.push_back(sp);
        }
        for (auto* sub: p->getSubPrograms()) {
            insertAccess(sub, cared);
        }
    }
}

Grammar * polygen::buildDefaultConditionGrammar(TypeList &inp_type, const PolyGenConfig &c, const ProgramList& extra_list) {
    auto* ig = buildIntGrammar(inp_type, c);
    auto* start = new NonTerminal("start", TBOOL);
    // start->rule_list.push_back(new SemanticsRule(semantics::string2Semantics("=="), {ig->start_symbol, ig->start_symbol}));
    start->rule_list.push_back(new SemanticsRule(semantics::string2Semantics("<="), {ig->start_symbol, ig->start_symbol}));
    start->rule_list.push_back(new SemanticsRule(semantics::string2Semantics("=="), {ig->start_symbol, ig->start_symbol}));
    start->rule_list.push_back(new SemanticsRule(semantics::string2Semantics("<"), {ig->start_symbol, ig->start_symbol}));
    std::vector<NonTerminal*> symbol_list = {start};
    for (auto* symbol: ig->symbol_list) symbol_list.push_back(symbol);
    for (auto* p: extra_list) {
        if (program::isParam(p)) continue;
        ProgramList cared = {p}; insertAccess(p, cared);
        for (auto* sp: cared) {
            for (auto *symbol: symbol_list) {
                if (type::equal(sp->oup_type, symbol->type)) {
                    symbol->rule_list.push_back(new ExtraProgramRule(sp));
                }
            }
        }
    }
    return new Grammar(start, symbol_list);
}