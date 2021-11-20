//
// Created by pro on 2021/9/2.
//

#include "grammar.h"
#include <cassert>
#include <iostream>
#include <queue>
#include <set>

NonTerminal::NonTerminal(const std::string &_name, Type *_type): name(_name), type(_type) {}
std::string NonTerminal::toString() const {
    return name;// + "@" + type->getName();
}

Rule::Rule(const std::vector<NonTerminal *> &_param_list): param_list(_param_list) {}

SemanticsRule::SemanticsRule(Semantics *_semantics, const SymbolList &_param_list): Rule(_param_list), semantics(_semantics) {}
std::string SemanticsRule::toString() const {
    std::string res = semantics->name;
    if (param_list.empty()) return res;
    for (int i = 0; i < param_list.size(); ++i) {
        res += (i ? ", ": "(") + param_list[i]->toString();
    }
    return res + ")";
}
SemanticsProgram * SemanticsRule::buildProgram(const ProgramList &sub_list) {
    return new SemanticsProgram(semantics, sub_list);
}

LambdaRule::LambdaRule(const ParamList &_inp_list, NonTerminal *content): inp_list(_inp_list), Rule({content}) {}
std::string LambdaRule::toString() const {
    std::string head = "\\(";
    for (int i = 0; i < inp_list.size(); ++i) {
        if (i) head += ", ";
        head += inp_list[i].first + "@" + inp_list[i].second->getName();
    }
    head += ")";
    return head + " " + param_list[0]->toString();
}
LambdaProgram * LambdaRule::buildProgram(const ProgramList &sub_list) {
    return new LambdaProgram(inp_list, sub_list[0]);
}

AccessRule::AccessRule(NonTerminal *_prod, int _id): Rule({_prod}), prod(_prod), id(_id) {}
std::string AccessRule::toString() const {
    return prod->toString() + "." + std::to_string(id);
}
AccessProgram * AccessRule::buildProgram(const ProgramList &sub_list) {
    return new AccessProgram(sub_list[0], id);
}

CmpRule::CmpRule(CmpType _type, NonTerminal *_key): Rule({_key}), type(_type), key(_key) {}
std::string CmpRule::toString() const {
    return cmp::cmp2String(type) + ":" + key->toString();
}
CmpProgram * CmpRule::buildProgram(const ProgramList &sub_list) {
    return new CmpProgram(type, sub_list[0]);
}

ExtraProgramRule::ExtraProgramRule(Program *_p): Rule({}), p(_p) {}
std::string ExtraProgramRule::toString() const {
    return p->toString();
}
Program * ExtraProgramRule::buildProgram(const ProgramList &sub_list) {
    return p->copy();
}

void Grammar::indexSymbols() const {
    for (int i = 0; i < symbol_list.size(); ++i) symbol_list[i]->id = i;
}
Grammar::Grammar(NonTerminal *_start_symbol, const SymbolList &_symbol_list): start_symbol(_start_symbol), symbol_list(_symbol_list) {
    std::unordered_map<std::string, int> symbol_name_map;
    for (int i = 0; i < symbol_list.size(); ++i) {
        assert(!symbol_name_map.count(symbol_list[i]->name));
        symbol_name_map[symbol_list[i]->name] = i;
    }
    assert(symbol_name_map.count(start_symbol->name));
    int where = symbol_name_map[start_symbol->name];
    std::swap(symbol_list[where], symbol_list[0]);
}

void Grammar::print() const {
    for (auto* s: symbol_list) {
        std::cout << s->toString() << ":" << std::endl;
        for (auto* r: s->rule_list) {
            std::cout << "\t" << r->toString() << std::endl;
        }
    }
}

namespace {
    struct SymbolInfo {
        Type* type;
        TypeList tmp_list;
        int depth;
        std::string name;
        std::string toString() const {
            return "S" + std::to_string(depth) + "@(" + type->getName() + ")@" + type::typeList2String(tmp_list);
        }
        SymbolInfo(Type* _type, const TypeList& _tmp_list, int _depth): type(_type), tmp_list(_tmp_list), depth(_depth) {
            name = toString();
        }
        int operator < (const SymbolInfo& s) const {
            return name < s.name;
        }
        NonTerminal* buildSymbol() const {
            return new NonTerminal(name, type);
        }
    };

    std::string getTmpVarName(int id) {
        return "tmp" + std::to_string(id);
    }

    std::map<SymbolInfo, NonTerminal*> symbol_cache;
    std::queue<SymbolInfo> symbol_queue;
    NonTerminal* insertSymbol(const SymbolInfo& s) {
        if (symbol_cache.count(s)) return symbol_cache[s];
        auto* res = s.buildSymbol();
        symbol_cache[s] = res;
        symbol_queue.push(s);
        return res;
    }

    void collectTypeVars(Type* type, std::set<std::string>& var_set) {
        if (type->type == T_VAR) {
            auto* vt = dynamic_cast<TypeVar*>(type);
            var_set.insert(vt->name);
        }
        for (auto* sub_type: type->param) {
            collectTypeVars(sub_type, var_set);
        }
    }

    bool checkValid(Type* type, const TypeList& basic_types) {
        if (type->type == T_ARROW) {
            for (auto* param: type->param) {
                if (!checkValid(param, basic_types)) return false;
            }
            return true;
        }
        for (auto* basic_type: basic_types) {
            if (type::equal(type, basic_type)) return true;
        }
        return false;
    }

    void _getPossibleConcretion(int pos, const std::vector<std::string>& var_list, std::map<std::string, Type*>& substitution,
            Semantics* basic_semantics, const TypeList& basic_types, Type* symbol_type, std::vector<Semantics*>& res) {
        if (pos == var_list.size()) {
            auto* oup_type = type::applySubstitution(basic_semantics->oup_type, substitution);
            if (!type::equal(oup_type, symbol_type)) return;
            TypeList inp_list;
            for (auto* type: basic_semantics->inp_type_list) {
                auto* cur = type::applySubstitution(type, substitution);
                if (!checkValid(cur, basic_types)) return;
                inp_list.push_back(cur);
            }
            res.push_back(basic_semantics->concretion(inp_list, oup_type));
            return;
        }
        for (auto* type: basic_types) {
            substitution[var_list[pos]] = type;
            _getPossibleConcretion(pos + 1, var_list, substitution, basic_semantics, basic_types, symbol_type, res);
        }
    }


    std::vector<Semantics*> getPossibleConcretion(Semantics* s, const TypeList& basic_types, Type* oup_type) {
        std::set<std::string> var_set;
        for (auto* inp_type: s->inp_type_list) collectTypeVars(inp_type, var_set);
        collectTypeVars(s->oup_type, var_set);
        std::vector<Semantics*> res;
        std::map<std::string, Type*> substitution;
        std::vector<std::string> var_list;
        for (const auto& var_name: var_set) var_list.push_back(var_name);
        _getPossibleConcretion(0, var_list, substitution, s, basic_types, oup_type, res);
        return res;
    }

    void removeEmptySymbol(std::vector<NonTerminal*>& symbol_list) {
        std::set<std::string> removed_set;
        bool is_changed = true;
        while (is_changed) {
            is_changed = false;
            for (auto* s: symbol_list) {
                if (removed_set.find(s->name) != removed_set.end()) continue;
                int now = 0;
                for (auto* r: s->rule_list) {
                    bool is_empty = false;
                    for (auto* subs: r->param_list) {
                        if (removed_set.find(subs->name) != removed_set.end()) {
                            is_empty = true; break;
                        }
                    }
                    if (!is_empty) s->rule_list[now++] = r;
                }
                s->rule_list.resize(now);
                if (s->rule_list.empty()) {
                    removed_set.insert(s->name);
                    is_changed = true;
                }
            }
        }
        int now = 0;
        for (auto* s: symbol_list) {
            if (removed_set.find(s->name) == removed_set.end()) symbol_list[now++] = s;
        }
        symbol_list.resize(now);
    }
}

Grammar * grammar::buildGrammar(const GrammarConfig& config) {
    symbol_cache.clear();
    while (!symbol_queue.empty()) symbol_queue.pop();
    auto* start_symbol = insertSymbol(SymbolInfo(config.oup_type, {}, 0));
    while (!symbol_queue.empty()) {
        auto info = symbol_queue.front();
        symbol_queue.pop();
        auto* symbol = symbol_cache[info];
        if (symbol->type->type == T_ARROW) {
            if (info.depth == config.lambda_depth) continue;
            auto inp_list = type::extractInpListFromArrow(symbol->type);
            auto tmp_list = info.tmp_list;
            if (tmp_list.size() + inp_list.size() > config.tmp_num) continue;
            auto* oup = type::extractOupFromArrow(symbol->type);
            ParamList param_list;
            for (auto* inp_type: inp_list) {
                param_list.emplace_back(getTmpVarName(tmp_list.size()), inp_type);
                tmp_list.push_back(inp_type);
            }
            SymbolInfo sub_info(oup, tmp_list, info.depth + 1);
            symbol->rule_list.push_back(new LambdaRule(param_list, insertSymbol(sub_info)));
        } else {
            // build access rules
            for (auto* basic_type: config.basic_types) {
                if (basic_type->type == T_PROD) {
                    for (int i = 0; i < basic_type->param.size(); ++i) {
                        if (type::equal(basic_type->param[i], symbol->type)) {
                            SymbolInfo sub_info(basic_type, info.tmp_list, info.depth);
                            symbol->rule_list.push_back(new AccessRule(insertSymbol(sub_info), i + 1));
                        }
                    }
                }
            }
            // build semantics rules
            for (const auto& data: config.const_list) {
                if (type::equal(info.type, data.getType())) {
                    auto* cs = new ConstSemantics(data);
                    symbol->rule_list.push_back(new SemanticsRule(cs, {}));
                }
            }
            for (int i = 0; i < config.var_list.size(); ++i) {
                if (type::equal(info.type, config.var_list[i])) {
                    auto* ps = new ParamSemantics(i, config.var_list[i]);
                    symbol->rule_list.push_back(new SemanticsRule(ps, {}));
                }
            }
            for (int i = 0; i < info.tmp_list.size(); ++i) {
                if (type::equal(info.type, info.tmp_list[i])) {
                    auto* ts = new TmpSemantics(getTmpVarName(i), info.tmp_list[i]);
                    symbol->rule_list.push_back(new SemanticsRule(ts, {}));
                }
            }
            for (const auto& op_name: config.op_list) {
                auto* os = semantics::string2Semantics(op_name);
                // std::cout << "op " << op_name << std::endl;
                std::vector<Semantics*> s_list = getPossibleConcretion(os, config.basic_types, symbol->type);
                for (auto* semantics: s_list) {
                    SymbolList sub_symbols;
                    // std::cout << "  " << type::typeList2String(semantics->inp_type_list) << " " << semantics->oup_type->getName() << std::endl;
                    for (auto* inp_type: semantics->inp_type_list) {
                        SymbolInfo sub_info(inp_type, info.tmp_list, info.depth);
                        sub_symbols.push_back(insertSymbol(sub_info));
                    }
                    symbol->rule_list.push_back(new SemanticsRule(semantics, sub_symbols));
                }
            }
        }
    }


    SymbolList symbol_list;
    for (auto& info: symbol_cache) symbol_list.push_back(info.second);

    removeEmptySymbol(symbol_list);
    if (symbol_list.empty()) symbol_list.push_back(start_symbol);
    return new Grammar(start_symbol, symbol_list);
}

namespace {
    void insertType(TypeList& x, Type* t) {
        for (auto* tx: x) {
            if (type::equal(tx, t)) return;
        }
        x.push_back(t);
    }

    void mergeTypeList(TypeList& x, const TypeList& y) {
        for (auto* t: y) insertType(x, t);
    }

    void _mergeTypeStorage(int pos, const std::vector<TypeList>& type_storage, TypeList& tmp, std::vector<TypeList>& res) {
        if (pos == type_storage.size()) {
            res.push_back(tmp); return;
        }
        for (auto* t: type_storage[pos]) {
            tmp[pos] = t;
            _mergeTypeStorage(pos + 1, type_storage, tmp, res);
        }
    }

    std::vector<TypeList> mergeTypeStorage(const std::vector<TypeList>& type_storage) {
        TypeList tmp(type_storage.size());
        std::vector<TypeList> res;
        _mergeTypeStorage(0, type_storage, tmp, res);
        return res;
    }

    TypeList extractSimpleType(Type* type) {
        TypeList res = {type};
        if (type->type == T_PROD) {
            for (auto* content: type->param) {
                mergeTypeList(res, extractSimpleType(content));
            }
            return res;
        }
        std::vector<TypeList> type_storage;
        for (auto* sub_type: type->param) {
            auto sub_res = extractSimpleType(sub_type);
            mergeTypeList(res, sub_res);
            type_storage.push_back(sub_res);
        }
        std::vector<TypeList> merge_res = mergeTypeStorage(type_storage);
        for (auto& param_list: merge_res) {
            auto* new_t = new Type(type->type, param_list);
            insertType(res, new_t);
        }
        return res;
    }
}

Grammar * grammar::buildDefaultGrammar(const TypeList &var_list, Type *oup_type) {
    GrammarConfig c;
    c.const_list = {0, 1};
    c.op_list = {"+", "neg", "fold", "lmatch", "lmove", "bc", "bmatch", "bmovel", "bmover", "bfold"};
    c.var_list = var_list;
    c.oup_type = oup_type;
    c.basic_types = {};
    mergeTypeList(c.basic_types, extractSimpleType(type::removeLimit(oup_type)));
    for (auto* inp_type: var_list) {
        mergeTypeList(c.basic_types, extractSimpleType(type::removeLimit(inp_type)));
    }
    return buildGrammar(c);
}

NonTerminal * grammar::getDefaultSymbolForType(Type *type) {
    assert(type::equal(type, TINT) || type::equal(type, TBOOL));
    return new NonTerminal(type->getName() + "_expr", type);
}
