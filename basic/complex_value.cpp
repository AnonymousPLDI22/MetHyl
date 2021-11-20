//
// Created by pro on 2021/9/1.
//

#include "complex_value.h"
#include "semantics.h"
#include <cassert>

bool ListValue::equal(Value* v) const {
    auto* lv = dynamic_cast<ListValue*>(v);
    if (!lv) return false;
    if (lv->value.size() != value.size()) return false;
    for (int i = 0; i < value.size(); ++i) {
        if (lv->value[i] != value[i]) return false;
    }
    return true;
}


ListValue::ListValue(const std::vector<Data> &_value, Type* _type): Value(_type), value(_value) {
    if (type == nullptr) {
        assert(!value.empty()); type = new Type(T_LIST, {value[0].getType()});
    }
#ifdef DEBUG
    assert(type->type == T_LIST);
    for (auto& d: value) assert(type::equal(d.getType(), type->param[0]));
#endif
}
std::string ListValue::toString() const {
    std::string res = "[";
    for (int i = 0; i < value.size(); ++i) {
        if (i) res += ",";
        res += value[i].toString();
    }
    return res + "]";
}

ProdValue::ProdValue(const std::vector<Data> &_value, Type *_type): Value(_type), value(_value) {
    if (type == nullptr) {
        TypeList type_list;
        for (const auto& v: value) type_list.push_back(v.getType());
        type = new Type(T_PROD, type_list);
    }
#ifdef DEBUG
    assert(type->type == T_PROD && type->param.size() == value.size());
    for (int i = 0; i < value.size(); ++i) assert(type::equal(value[i].getType(), type->param[i]));
#endif
}
std::string ProdValue::toString() const {
    std::string res = "(";
    for (int i = 0; i < value.size(); ++i) {
        if (i) res += ",";
        res += value[i].toString();
    }
    return res + ")";
}
bool ProdValue::equal(Value *v) const {
    auto* pv = dynamic_cast<ProdValue*>(v);
    if (!pv) return false;
    if (value.size() != pv->value.size()) return false;
    for (int i = 0; i < value.size(); ++i) {
        if (value[i] != pv->value[i]) return false;
    }
    return true;
}

namespace {
    TypeList buildTypeListFromSemantics(Semantics* semantics) {
        TypeList res = semantics->inp_type_list;
        res.push_back(semantics->oup_type);
        return res;
    }
}

ArrowValue::ArrowValue(Semantics *_semantics, bool _is_anonymous):
        semantics(_semantics), is_anonymous(_is_anonymous), Value(new Type(T_ARROW, buildTypeListFromSemantics(_semantics))) {
}
std::string ArrowValue::toString() const {return semantics->name;}
ArrowValue::~ArrowValue() noexcept {
    if (is_anonymous) delete semantics;
}

BTreeValue::BTreeValue(const Data &_v, const Data &_l, const Data &_r, class ::Type *_type): Value(_type), l(_l), r(_r), v(_v) {
    assert(l.isNull() == r.isNull());
    if (!type) {
        assert(!l.isNull());
        type = l.getType();
    }
#ifdef DEBUG
    assert(type->type == T_BTREE);
    if (!l.isNull()) {
        assert(type::equal(l.getType(), type) && type::equal(r.getType(), type));
        assert(type::equal(v.getType(), type->param[0]));
    } else {
        assert(type::equal(v.getType(), type->param[1]));
    }
#endif
}

bool BTreeValue::isLeaf() const {return l.isNull();}
int BTreeValue::size() const {
    if (isLeaf()) return 0;
    return l.getBTree()->size() + r.getBTree()->size() + 1;
}

std::string BTreeValue::toString() const {
    if (isLeaf()) {
        return v.toString();
    } else if (v.isNull()) {
        return "{" + l.toString() + ", " + r.toString() + "}";
    } else {
        return "{" + v.toString() + ", " + l.toString() + ", " + r.toString() + "}";
    }
}

namespace {
    bool btreeEqual(const BTreeValue* x, const BTreeValue* y) {
        if (x->isLeaf() != y->isLeaf()) return false;
        if (x->isLeaf()) return x->v == y->v;
        return x->v == y->v && btreeEqual(x->l.getBTree(), y->l.getBTree()) && btreeEqual(x->r.getBTree(), y->r.getBTree());
    }
}

bool BTreeValue::equal(Value *v) const {
    auto* bv = dynamic_cast<BTreeValue*>(v);
    if (!bv) return false;
    if (!type::equal(v->getType(), getType())) return false;
    return btreeEqual(this, bv);
}