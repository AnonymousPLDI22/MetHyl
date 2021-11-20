//
// Created by pro on 2021/8/23.
//

#ifndef DPSYNTHESISNEW_TYPE_H
#define DPSYNTHESISNEW_TYPE_H

#include <string>
#include <vector>
#include <map>

enum TypeType {
    T_INT, T_BOOL, T_PROD, T_LIST, T_BTREE, T_ARROW, T_VAR, T_VOID, T_SUM
};

class Type {
public:
    TypeType type;
    std::vector<Type*> param;
    Type(const TypeType& _type, const std::vector<Type*>& _param);
    virtual std::string getName() const;
    virtual ~Type() = default;
};

class TypeVar: public Type {
public:
    std::string name;
    TypeVar(const std::string& _name): Type(T_VAR, {}), name(_name) {}
    virtual std::string getName() const {return name;}
    virtual ~TypeVar() = default;
};

class LimitedType: public Type {
public:
    virtual LimitedType* squeeze(double alpha, const std::vector<Type*>& sub_list) = 0;
    LimitedType(const TypeType& _type, const std::vector<Type*>& _param): Type(_type, _param) {}
};

class LimitedInt: public LimitedType {
public:
    int l, r;
    LimitedInt(int _l, int _r);
    virtual std::string getName() const;
    virtual ~LimitedInt() = default;
    virtual LimitedType * squeeze(double alpha, const std::vector<Type *> &sub_list);
};

class SizeLimitedDS: public LimitedType {
public:
    int size;
    SizeLimitedDS(const TypeType& _type, int _size, const std::vector<Type*>& param);
    virtual std::string getName() const;
    virtual ~SizeLimitedDS() = default;
    virtual LimitedType * squeeze(double alpha, const std::vector<Type *> &sub_list);
};

typedef std::vector<Type*> TypeList;
typedef std::map<std::string, Type*> Substitution;

#define InitType(name) extern Type* getT##name();
namespace type {
    InitType(INT)
    InitType(BOOL)
    InitType(VOID)
    InitType(A)
    InitType(B)
    InitType(C)
    bool equal(Type* x, Type* y);
    bool checkUnify(const TypeList& x, const TypeList& y, Substitution& sigma);
    Type* applySubstitution(Type* x, const Substitution& sigma);
    TypeList extractInpListFromArrow(Type* x);
    Type* extractOupFromArrow(Type* x);
    Type* fillTypeAbstraction(Type* x, Type* type);
    Type* getPure(Type* type);
    std::string typeList2String(const TypeList& type_list);
    Type* removeLimit(Type* t);
    TypeList unfoldProdType(Type* t);
    TypeList mergeTypeList(const TypeList& x, const TypeList& y);
    bool isIntProduct(Type* type);
    bool isIntVarProduct(Type* type);
}

#define TINT type::getTINT()
#define TBOOL type::getTBOOL()
#define TVOID type::getTVOID()
#define TVARA type::getTA()
#define TVARB type::getTB()
#define TVARC type::getTC()

#endif //DPSYNTHESISNEW_TYPE_H
