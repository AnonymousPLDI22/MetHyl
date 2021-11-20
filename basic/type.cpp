//
// Created by pro on 2021/8/23.
//

#include "type.h"
#include <cassert>
#include <map>
#include <iostream>
#include "config.h"

namespace {
    bool checkArity(const TypeType& type, int num) {
        if (type == T_INT || type == T_BOOL || type == T_VAR) return num == 0;
        if (type == T_LIST) return num == 1;
        if (type == T_BTREE) return num == 2;
        if (type == T_ARROW || type == T_SUM) return num >= 1;
        if (type == T_PROD) return num >= 0;
        return true;
    }

    std::map<TypeType, std::string> ds_type_name = {
            {T_LIST, "List"}, {T_BTREE, "BTree"}
    };

    bool isDataStructure(const TypeType& type) {
        return ds_type_name.count(type) > 0;
    }

    std::string mergeName(const std::string& type, const std::vector<Type*>& param, const std::string& merge_op = ",", int skip = 0) {
        auto res = type;
        for (int i = 0; i + skip < param.size(); ++i) {
            res += (i ? merge_op : "(") + param[i]->getName();
        }
        return res + ")";
    }
}

Type::Type(const TypeType &_type, const std::vector<Type *>& _param): type(_type), param(_param) {
    assert(checkArity(type, param.size()));
}

std::string Type::getName() const {
    if (type == T_INT) return "Int";
    if (type == T_BOOL) return "Bool";
    if (type == T_VOID) return "Void";
    if (type == T_PROD) return mergeName("", param, "*");
    if (type == T_SUM) return mergeName("", param, "+");
    if (isDataStructure(type)) return mergeName(ds_type_name[type], param);
    if (type == T_ARROW) return mergeName("", param, ",", 1) + "->" + param[param.size() - 1]->getName();
    assert(false);
}

LimitedInt::LimitedInt(int _l, int _r): LimitedType(T_INT, {}), l(_l), r(_r) {
    assert(l <= r);
}
std::string LimitedInt::getName() const {
    return "Int[" + std::to_string(l) + "," + std::to_string(r) + "]";
}
LimitedType * LimitedInt::squeeze(double alpha, const std::vector<Type *> &sub_list) {
    assert(sub_list.empty());
    if (r - l + 1 <= config::KLimitedTypeRangeLimit) return new LimitedInt(l, r);
    alpha = std::max(alpha, 1.0 * config::KLimitedTypeRangeLimit / (r - l + 1));
    int new_l = int(l * alpha);
    int new_r = int(r * alpha);
    return new LimitedInt(new_l, new_r);
}

SizeLimitedDS::SizeLimitedDS(const TypeType &_type, int _size, const std::vector<Type *> &_param): LimitedType(_type, _param), size(_size) {
    assert(checkArity(type, param.size()) && isDataStructure(type));
    assert(size >= 0);
}
std::string SizeLimitedDS::getName() const {
    std::string name = ds_type_name[type] + "[" + std::to_string(size) + "]";
    return mergeName(name, param);
}
LimitedType * SizeLimitedDS::squeeze(double alpha, const std::vector<Type *> &sub_list) {
    assert(param.size() == sub_list.size());
    if (size <= config::KLimitedTypeRangeLimit) return new SizeLimitedDS(type, size, sub_list);
    alpha = std::max(alpha, 1.0 * config::KLimitedTypeRangeLimit / size);
    return new SizeLimitedDS(type, int(size * alpha), sub_list);
}

bool type::equal(Type *x, Type *y) {
    if (x->type != y->type) return false;
    if (x->param.size() != y->param.size()) return false;
    for (int i = 0; i < x->param.size(); ++i) {
        if (!equal(x->param[i], y->param[i])) return false;
    }
    return true;
}

namespace {
    bool checkConcreteType(Type* x) {
        if (x->type == T_VAR) return false;
        for (auto* sub_type: x->param) {
            if (!checkConcreteType(sub_type)) return false;
        }
        return true;
    }

    bool tryUnify(Type* x, Type* y, Substitution& sigma) {
        if (y->type == T_VAR) std::swap(x, y);
        if (x->type == T_VAR) {
            assert(checkConcreteType(y));
            auto x_name = x->getName();
            if (sigma.count(x_name) && !type::equal(sigma[x_name], y)) return false;
            sigma[x_name] = y;
            return true;
        }
        if (x->type != y->type || x->param.size() != y->param.size()) return false;
        for (int i = 0; i < x->param.size(); ++i) {
            if (!tryUnify(x->param[i], y->param[i], sigma)) return false;
        }
        return true;
    }
}

// Partial support
bool type::checkUnify(const TypeList &x, const TypeList &y, Substitution &sigma) {
    assert(x.size() == y.size());
    for (int i = 0; i < x.size(); ++i) {
        if (!tryUnify(x[i], y[i], sigma)) return false;
    }
    return true;
}

Type * type::applySubstitution(Type *x, const Substitution &sigma) {
    if (checkConcreteType(x)) return x;
    if (x->type == T_VAR) {
        auto x_name = x->getName();
        assert(sigma.count(x_name));
        return sigma.find(x_name)->second;
    }
    std::vector<Type*> new_param;
    for (auto* p: x->param) {
        new_param.push_back(applySubstitution(p, sigma));
    }
    return new Type(x->type, new_param);
}

TypeList type::extractInpListFromArrow(Type *x) {
    assert(x->type == T_ARROW);
    TypeList res;
    for (int i = 0; i + 1 < x->param.size(); ++i) {
        res.push_back(x->param[i]);
    }
    return res;
}

Type* type::extractOupFromArrow(Type *x) {
    assert(x->type == T_ARROW);
    return x->param[x->param.size() - 1];
}

Type * type::getTINT() {
    static Type* t = nullptr;
    if (t) return t; else return t = new Type(T_INT, {});
}
Type * type::getTBOOL() {
    static Type* t = nullptr;
    if (t) return t; else return t = new Type(T_BOOL, {});
}
Type * type::getTVOID() {
    static Type* t = nullptr;
    if (t) return t; else return t = new Type(T_VOID, {});
}
Type * type::getTA() {
    static Type* t = nullptr;
    if (t) return t; else return t = new TypeVar("a");
}
Type * type::getTB() {
    static Type* t = nullptr;
    if (t) return t; else return t = new TypeVar("b");
}
Type * type::getTC() {
    static Type* t = nullptr;
    if (t) return t; else return t = new TypeVar("c");
}


Type * type::fillTypeAbstraction(Type *x, Type* type) {
    if (checkConcreteType(x)) return x;
    if (x->type == T_VAR) return type;
    TypeList type_list;
    for (auto* t: x->param) {
        type_list.push_back(fillTypeAbstraction(t, type));
    }
    return new Type(x->type, type_list);
}

std::string type::typeList2String(const TypeList &type_list) {
    std::string res = "[";
    for (int i = 0; i < type_list.size(); ++i) {
        if (i) res += ",";
        res += type_list[i]->getName();
    }
    return res + "]";
}

Type * type::removeLimit(Type *t) {
    TypeList param_list;
    for (auto* sub_t: t->param) {
        param_list.push_back(removeLimit(sub_t));
    }
    return new Type(t->type, param_list);
}

TypeList type::unfoldProdType(Type *t) {
    if (t->type == T_PROD) {
        return t->param;
    }
    return {t};
}

TypeList type::mergeTypeList(const TypeList &x, const TypeList &y) {
    auto res = x;
    for (auto* t: y) res.push_back(t);
    return res;
}

bool type::isIntProduct(Type *type) {
    if (type->type == T_INT) return true;
    if (type->type == T_PROD) {
        for (auto* sub: type->param) {
            if (!isIntProduct(sub)) return false;
        }
        return true;
    }
    return false;
}

bool type::isIntVarProduct(Type *type) {
    if (type->type == T_INT || type->type == T_VAR) return true;
    if (type->type == T_PROD) {
        for (auto* sub: type->param) {
            if (!isIntVarProduct(sub)) return false;
        }
        return true;
    }
    return false;
}

Type* type::getPure(Type* type) {
    TypeList param_list;
    for (auto* sub_type: type->param) param_list.push_back(getPure(sub_type));
    return new Type(type->type, param_list);
}