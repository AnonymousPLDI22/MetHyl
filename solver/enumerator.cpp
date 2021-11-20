//
// Created by pro on 2021/9/7.
//

#include "enumerator.h"
#include "time_guard.h"
#include <iostream>
#include <set>

bool PointVerifier::isValid(Program *program) {
    for (auto *example: example_list) {
        if (program->run(example->first) != example->second) return false;
    }
    return true;
}

EnumConfig::EnumConfig(Verifier *_v, Optimizer *_o, int _num_limit, int _size_limit): v(_v), o(_o),
    num_limit(_num_limit), size_limit(_size_limit) {
}

bool OEOptimizer::isDuplicated(int symbol_id, Program *program) {
    if (!program::extractUsedTmps(program).empty()) return false;
    if (guard && guard->isTimeOut()) throw TimeOutError();
    /*bool isp = false;
    if (program->toString() == "lmove(Param@1,Param@0.1,0,Param@2)") {
        isp = true;
        std::cout << "find" << std::endl;
    }*/
    DataList oup_list;
    for (auto& example: test_inp_list) {
        try {
            oup_list.push_back(program->run(example));
        } catch (SemanticsError& e) {
            //if (isp) std::cout << "move " << program->toString() << " " << data::dataList2String(example) << std::endl;
            return true;
        }
    }
    if (program->oup_type->type == T_ARROW) return false;
    if (program->oup_type->type == T_INT) {
        for (auto& oup: oup_list) if (std::abs(oup.getInt()) > int_limit) {
            return true;
        }
    }
    auto feature = std::to_string(symbol_id) + "@" + data::dataList2String(oup_list);
    if (feature_set.find(feature) == feature_set.end()) {
        feature_set[feature] = program->toString();
        //if (isp) std::cout << "accept " << program->toString() << " " << feature_set[feature] << " " << feature << std::endl;
        return false;
    }
    //if (isp) std::cout << "move " << program->toString() << " " << feature_set[feature] << " " << feature << std::endl;
    /*if (program->toString() == "lmove(Param@1,Param@0.1,Param@2,Param@0.2)") {
        std::cout << feature << std::endl;
        std::cout << symbol_id << " " << "ffind" << " " << feature_set[feature] << std::endl;
    }*/
    return true;
}

bool OEOptimizer::isAccept(Program *program) {
    //if (program->toString() == "lmove(Param@1,Param@0.1,Param@2,Param@0.2)") std::cout << "afind" << std::endl;
    return true;
}

void OEOptimizer::clear() {
    feature_set.clear();
}

bool ValidOptimizer::isDuplicated(int symbol_id, Program *program) {
    if (!program::extractUsedTmps(program).empty()) return false;
    for (auto& example: storage) {
        try {
            program->run(example);
        } catch (SemanticsError& e) {
            return true;
        }
    }
    return false;
}

namespace {
    void _splitSize(int pos, int rem, const std::vector<std::vector<int>>& pool, std::vector<int>& tmp, std::vector<std::vector<int>>& res) {
        if (pos == tmp.size()) {
            if (rem == 0) res.push_back(tmp); return;
        }
        for (int s: pool[pos]) {
            if (s <= rem) {
                tmp[pos] = s;
                _splitSize(pos + 1, rem - s, pool, tmp, res);
            }
        }
    }

    std::vector<std::vector<int>> splitSize(int size, const std::vector<std::vector<int>>& pool) {
        std::vector<std::vector<int>> res;
        std::vector<int> tmp(pool.size());
        _splitSize(0, size, pool, tmp, res);
        return res;
    }

    void _mergeProgram(int pos, const ProgramStorage& program_storage, ProgramList& tmp, ProgramList& res, Rule* r, int rem) {
        if (pos == tmp.size()) {
            res.push_back(r->buildProgram(tmp));
            return;
        }
        for (auto* sub: program_storage[pos]) {
            tmp[pos] = sub;
            _mergeProgram(pos + 1, program_storage, tmp, res, r, rem);
            if (int(res.size()) >= rem) return;
        }
    }

    ProgramList mergeProgram(const ProgramStorage& program_storage, Rule* r, int rem) {
        ProgramList res, tmp(program_storage.size());
        _mergeProgram(0, program_storage, tmp, res, r, rem);
        return res;
    }
}

namespace {
    void clearMidResult(const ProgramList& pl, const std::vector<ProgramStorage>& psl) {
        for (auto* p: pl) p->delRef();
        for (auto& ps: psl) for (auto& p_list: ps) for (auto* p: p_list) p->delRef();
    }
}

std::vector<Program *> enumerate::synthesis(Grammar* grammar, const EnumConfig &c) {
    auto* v = c.v; auto* o = c.o; o->clear();
    ProgramList result;
    std::vector<ProgramStorage> storage_list;
    grammar->indexSymbols();
    for (auto* _: grammar->symbol_list) {
        storage_list.push_back({{}});
    }
    int calc_num = 0;
    for (int size = 1; size <= c.size_limit && calc_num < c.calc_num; ++size) {
        for (auto* symbol: grammar->symbol_list) {
            storage_list[symbol->id].emplace_back();
            for (auto* rule: symbol->rule_list) {
                std::vector<std::vector<int>> pool;
                for (auto* param_symbol: rule->param_list) {
                    ++calc_num;
                    int param_id = param_symbol->id;
                    std::vector<int> choice;
                    for (int i = 1; i < size; ++i) {
                        if (!storage_list[param_id][i].empty()) choice.push_back(i);
                    }
                    pool.push_back(choice);
                }
                auto split_list = splitSize(size - 1, pool);
                for (auto& split: split_list) {
                    ProgramStorage candidate;
                    for (int i = 0; i < split.size(); ++i) {
                        candidate.push_back(storage_list[rule->param_list[i]->id][split[i]]);
                    }
                    auto merged_programs = mergeProgram(candidate, rule, c.calc_num - calc_num);
                    for (auto* program: merged_programs) {
                        ++calc_num;
                        if (calc_num > c.calc_num) {
                            // std::cout << "res size " << size << " " << merged_programs[0]->toString() << std::endl;
                            clearMidResult(merged_programs, storage_list);
                            return result;
                        }
                        if (o->isDuplicated(symbol->id, program)) continue;
                        storage_list[symbol->id][size].push_back(program->copy());
                        if (symbol->id == 0 && v->isValid(program) && o->isAccept(program)) {
                            result.push_back(program->copy());
                            if (result.size() >= c.num_limit) {
                                clearMidResult(merged_programs, storage_list);
                                return result;
                            }
                        }
                    }
                    clearMidResult(merged_programs, {});
                }
            }
        }
    }
    clearMidResult({}, storage_list);
    return result;
}