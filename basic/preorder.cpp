//
// Created by pro on 2021/9/14.
//

#include "preorder.h"
#include "config.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <unordered_set>

void PreOrder::print() const {
    std::cout << toString() << std::endl;
}

DataList PreOrder::getMaximal(const DataList &data_list, const DataList &env) const {
    DataList res(data_list.size());
    int size = 0;
    for (auto& data: data_list) {
        int now = 0, flag = 0;
        for (int i = 0; i < size; ++i) {
            if (leq(data, res[i], env)) {
                flag = 1; break;
            } else if (!leq(res[i], data, env)) {
                res[now++] = res[i];
            }
        }
        assert(flag == 0 || now == size);
        if (flag == 0) {
            res[now++] = data;
        }
        size = now;
    }
    res.resize(size);
    return res;
}

bool PreOrder::coveredBy(const DataList &x, const DataList &y, const DataList& env) const {
    for (auto& x_d: x) {
        bool is_covered = false;
        for (auto& y_d: y) {
            if (leq(x_d, y_d, env)) {
                is_covered = true; break;
            }
        }
        if (!is_covered) return false;
    }
    return true;
}

std::string CmpPreOrder::toString() const {
    ProgramList prog_list;
    for (auto* cmp: cmp_list) prog_list.push_back(cmp);
    return "Cmp:" + program::programList2String(prog_list);
}

int CmpPreOrder::leq(const Data &x, const Data &y, const DataList &env) const {
    for (const auto& unit: cmp_list) {
        if (!unit->run({x}, {y}, env).getBool()) return false;
    }
    return true;
}

CmpPreOrder * CmpPreOrder::insert(CmpProgram *cmp) const {
    ProgramList new_list = {cmp};
    for (auto* p: cmp_list) new_list.push_back(p);
    return new CmpPreOrder(new_list);
}

CmpPreOrder::CmpPreOrder(const std::vector<Program*> &_cmp_list) {
    for (auto* program: _cmp_list) {
        auto* cp = dynamic_cast<CmpProgram*>(program);
        assert(cp);
        if (cp->type == GEQ) {
            auto* new_key = new SemanticsProgram(semantics::string2Semantics("neg"), {cp->key});
            cmp_list.push_back(new CmpProgram(LEQ, new_key));
        } else {
            cmp_list.push_back(cp);
        }
    }
}

namespace {
    template<class T>
    std::vector<T> reorder(const std::vector<T>& inp, const std::vector<int>& ind) {
        std::vector<T> res;
        for (auto& pos: ind) res.push_back(inp[pos]);
        return res;
    }
}

DataList CmpPreOrder::getMaximal(const DataList& data_list, const DataList &env) const {
    if (data_list.empty()) return {};
    DataStorage oup_storage;
    for (auto& inp: data_list) {
        DataList oup_list;
        for (auto* unit: cmp_list) {
            oup_list.push_back(unit->getValue({inp}, env));
        }
        oup_storage.push_back(oup_list);
    }
    // GetRange
    std::vector<int> range_list, min_list;
    for (int i = 0; i < cmp_list.size(); ++i) {
        int mi = config::KINF, ma = -config::KINF;
        for (int j = 0; j < data_list.size(); ++j) {
            int now = oup_storage[j][i].getInt();
            mi = std::min(mi, now); ma = std::max(ma, now);
        }
        range_list.push_back(ma - mi + 1);
        min_list.push_back(mi);
    }
    auto cmp = [=](int ind_1, int ind_2) {
        return range_list[ind_1] < range_list[ind_2];
    };
    std::vector<int> ind_list;
    for (int i = 0; i < cmp_list.size(); ++i) ind_list.push_back(i);
    std::sort(ind_list.begin(), ind_list.end(), cmp);
    range_list = reorder(range_list, ind_list);
    min_list = reorder(min_list, ind_list);
    for (auto& oup_list: oup_storage) {
        oup_list = reorder(oup_list, ind_list);
    }
    // Record
    std::vector<int> dim_size(range_list.size());
    int n = 1;
    for (int i = int(range_list.size()) - 1; i >= 0; --i) {
        dim_size[i] = n;
        n *= range_list[i];
    }
    /*std::cout << this->toString() << std::endl;
    std::cout << "inp " << data::dataList2String(data_list) << std::endl;
    std::cout << "oups:" << std::endl;
    for (auto& oup_list: oup_storage) std::cout << "  " << data::dataList2String(oup_list) << std::endl;
    for (int i = 0; i < range_list.size(); ++i) std::cout << range_list[i] << " "; std::cout << std::endl;
    for (int i = 0; i < range_list.size(); ++i) std::cout << dim_size[i] << " "; std::cout << std::endl;
    for (int i = 0; i < min_list.size(); ++i) std::cout << min_list[i] << " "; std::cout << std::endl;*/
    std::vector<int> best_ind(n);
    for (int i = 0; i < n; ++i) best_ind[i] = -1;
    for (int i = 0; i < data_list.size(); ++i) {
        int pos = 0;
        for (int j = 0; j < dim_size.size(); ++j) {
            pos += dim_size[j] * (oup_storage[i][j].getInt() - min_list[j]);
        }
        assert(pos >= 0 && pos < n);
        best_ind[pos] = i;
    }
    for (int i = 0; i < cmp_list.size(); ++i) {
        if (cmp_list[i]->type == EQ) continue;
        assert(cmp_list[i]->type == LEQ);
        int r_size = i ? dim_size[i - 1] : n;
        for (int r_pos = 0; r_pos < n; r_pos += r_size) {
            for (int l_pos = 0; l_pos < dim_size[i]; ++l_pos) {
                int pos = r_pos + l_pos + r_size - (dim_size[i] << 1);
                for (int j = range_list[i] - 2; j >= 0; --j, pos -= dim_size[i]) {
                    if (best_ind[j + dim_size[i]] != -1) {
                        best_ind[j] = best_ind[j + dim_size[i]];
                    }
                }
            }
        }
    }
    // Collect Answer
    std::vector<bool> is_remain(data_list.size());
    for (int i = 0; i < data_list.size(); ++i) is_remain[i] = false;
    for (int i = 0; i < n; ++i) if (best_ind[i] != -1) {
        assert(best_ind[i] >= 0 && best_ind[i] < data_list.size());
        is_remain[best_ind[i]] = true;
    }
    DataList res;
    for (int i = 0; i < data_list.size(); ++i) if (is_remain[i]) {
        res.push_back(data_list[i]);
    }
    return res;
}

int EqRelation::leq(const Data &x, const Data &y, const DataList &env) const {
    return getFeature(x, env) == getFeature(y, env);
}

DataList EqRelation::getMaximal(const DataList &data_list, const DataList &env) const {
    std::unordered_set<std::string> feature_set;
    DataList res;
    for (auto& d: data_list) {
        auto feature = getFeature(d, env);
        if (feature_set.find(feature) == feature_set.end()) continue;
        feature_set.insert(feature);
        res.push_back(d);
    }
    return res;
}

KeyEqRelation::KeyEqRelation(const std::vector<Program *> &cmp_list, bool _is_unfold): is_unfold(_is_unfold) {
    if (cmp_list.empty()) return;
    if (dynamic_cast<CmpProgram*>(cmp_list[0])) {
        for (auto* p: cmp_list) {
            auto* cp = dynamic_cast<CmpProgram*>(p);
            assert(cp);
            key_list.push_back(cp->key);
        }
    } else {
        key_list = cmp_list;
    }
}

std::string KeyEqRelation::toString() const {
    return "Eq:" + program::programList2String(key_list);
}

std::string KeyEqRelation::getFeature(const Data &x, const DataList &env) const {
    DataList inp;
    if (is_unfold) inp = data::unfoldProdData(x); else inp = {x};
    DataList feature_list;
    for (auto* key: key_list) {
        feature_list.push_back(key->run(inp, env));
    }
    return data::dataList2String(feature_list);
}

std::string IdRelation::getFeature(const Data &x, const DataList &env) const {
    return x.toString();
}