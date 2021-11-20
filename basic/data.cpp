//
// Created by pro on 2021/7/30.
//

#include "data.h"
#include "complex_value.h"
#include <cassert>
#include <iostream>

int Data::getInt() const {
    auto* it = dynamic_cast<IntValue*>(value);
#ifdef DEBUG
    assert(it);
#endif
    return it->getInt();
}

int Data::getBool() const {
    auto* bt = dynamic_cast<BoolValue*>(value);
#ifdef DEBUG
    assert(bt);
#endif
    return bt->getBool();
}

bool operator == (const Data& d1, const Data& d2) {
    return d1.value->equal(d2.value);
}

bool operator != (const Data& d1, const Data& d2) {
    return !(d1 == d2);
}

bool operator <= (const Data& d1, const Data& d2) {
#ifdef DEBUG
    assert(type::equal(d1.getType(), d2.getType()));
#endif
    return d1.getInt() <= d2.getInt();
}

ListValue * Data::getList() const {
    auto* lv = dynamic_cast<ListValue*>(value);
    assert(lv);
    return lv;
}

BTreeValue* Data::getBTree() const {
    auto* bv = dynamic_cast<BTreeValue*>(value);
    assert(bv);
    return bv;
}

std::string data::dataList2String(const DataList &data_list) {
    std::string s = "[";
    int flag = false;
    for (auto& data: data_list) {
        if (flag) s += ",";
        s += data.toString();
        flag = true;
    }
    return s + "]";
}

Semantics * Data::getSemantics() const {
    auto* sv = dynamic_cast<ArrowValue*>(value);
    assert(sv);
    return sv->semantics;
}

Data Data::accessProd(int ind) const {
    auto* pv = dynamic_cast<ProdValue*>(value);
    assert(pv);
    return pv->value[ind];
}

DataList Data::getProdContents() const {
    auto* pv = dynamic_cast<ProdValue*>(value);
    assert(pv);
    return pv->value;
}

namespace {
    template<typename T> void tryCombine(int pos, const std::vector<std::vector<T>>& storage, std::vector<T>& tmp,
            std::vector<std::vector<T>>& res) {
        if (pos == storage.size()) {
            res.push_back(tmp);
            return;
        }
        for (auto& v: storage[pos]) {
            tmp[pos] = v;
            tryCombine(pos + 1, storage, tmp, res);
        }
    }
}

DataStorage data::cartesianProduct(const DataStorage &separate_data) {
    DataList tmp(separate_data.size()); DataStorage result;
    tryCombine(0, separate_data, tmp, result);
    return result;
}

DataList data::unfoldProdData(const Data &d) {
    auto* type = d.getType();
    if (type->type == T_PROD) {
        return d.getProdContents();
    } else return {d};
}

DataList data::mergeDataList(const DataList &x, const DataList &y) {
    DataList res = x;
    for (auto& w: y) res.push_back(w);
    return res;
}