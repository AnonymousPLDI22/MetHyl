//
// Created by pro on 2021/8/23.
//

#ifndef DPSYNTHESISNEW_SEMANTICS_H
#define DPSYNTHESISNEW_SEMANTICS_H

#include "data.h"
#include <unordered_map>
#include <functional>
#include <exception>

typedef std::pair<int, Data> CollectUnit;
typedef std::vector<CollectUnit> CollectRes;

struct SemanticsError: public std::exception {
};

class ExecuteInfo {
protected:
    const DataList* param_value;
    std::unordered_map<std::string, Data> tmp;
public:
    CollectRes collect_res;
    ExecuteInfo(const DataList* _param_value): param_value(_param_value) {}
    Data operator [] (int k) const {return (*param_value)[k];}
    Data& operator [] (const std::string& name) {return tmp[name];}
    void collect(int tag, const Data& data) {collect_res.emplace_back(tag, data);}
    int paramNum() const {return param_value->size();}
};

class Semantics {
protected:
    void check(const DataList& inp_list) const;
    virtual Data _run(DataList&& inp, ExecuteInfo& info) const = 0;
    virtual Semantics* copy() const = 0;
public:
    std::string name;
    std::vector<Type*> inp_type_list;
    Type* oup_type;
    bool is_polymorphic;
    Semantics(const std::string& _name, const TypeList& _inp_type_list, Type* _oup_type, bool _is_polymorphic = false);
    virtual ~Semantics() = default;
    Semantics* concretion(const TypeList& inp_list, Type* oup);
    Semantics* concretion(const TypeList& inp_list);
    Data run(DataList&& inp, ExecuteInfo& info) const;
};

class ConstSemantics: public Semantics {
public:
    Data value;
    ConstSemantics(const Data& _value): value(_value), Semantics(_value.toString(), {}, _value.getType()) {}
    virtual ~ConstSemantics() = default;
protected:
    virtual Data _run(DataList&& inp, ExecuteInfo& info) const {return value;}
    virtual Semantics* copy() const;
};

class ParamSemantics: public Semantics {
public:
    int id;
    ParamSemantics(int _id, Type* _type): Semantics("Param@" + std::to_string(_id), {}, _type), id(_id) {}
    virtual ~ParamSemantics() = default;
protected:
    virtual Data _run(DataList&& inp, ExecuteInfo& info) const;
    virtual Semantics* copy() const;
};

class TmpSemantics: public Semantics {
public:
    std::string name;
    TmpSemantics(const std::string& _name, Type* _type): Semantics(_name, {}, _type), name(_name) {}
    virtual ~TmpSemantics() = default;
protected:
    virtual Data _run(DataList&& inp, ExecuteInfo& info) const;
    virtual Semantics* copy() const;
};

typedef std::function<Data(DataList&&, ExecuteInfo&)> SemanticsFunction;

class AnonymousSemantics: public Semantics {
public:
    SemanticsFunction f;
    AnonymousSemantics(const std::string& name, SemanticsFunction&& _f, const std::vector<Type*>& _inp_type_list, Type* _oup_type):
        Semantics(name, _inp_type_list, _oup_type), f(_f) {}
    virtual ~AnonymousSemantics() = default;
protected:
    virtual Data _run(DataList&& inp, ExecuteInfo& info) const {return f(std::move(inp), info);}
    virtual Semantics* copy() const;
};

#define DefineSemantics(name) \
class name ## Semantics: public Semantics {\
public: \
    name ## Semantics(); \
    virtual ~name ## Semantics() = default; \
protected: \
    virtual Data _run(DataList&& inp, ExecuteInfo& info) const; \
    virtual Semantics* copy() const {return new name ## Semantics();} \
};

DefineSemantics(Plus)
DefineSemantics(Minus)
DefineSemantics(Max)
DefineSemantics(Min)
DefineSemantics(Times)
DefineSemantics(Leq)
DefineSemantics(Eq)
DefineSemantics(Gq)
DefineSemantics(Neg)
DefineSemantics(Ite)
DefineSemantics(Collect)
DefineSemantics(Size)
DefineSemantics(Head)
DefineSemantics(Last)
DefineSemantics(Tail)
DefineSemantics(Cons)
DefineSemantics(Nil)
DefineSemantics(Map)
DefineSemantics(Fold)
DefineSemantics(Sum)
DefineSemantics(Zip)
DefineSemantics(Xor)
DefineSemantics(Dots)
DefineSemantics(Init)
DefineSemantics(Sqr)
DefineSemantics(Print)
DefineSemantics(And)
DefineSemantics(Or)
DefineSemantics(Take)
DefineSemantics(Drop)
DefineSemantics(Access)
DefineSemantics(LMatch)
DefineSemantics(LMove)
DefineSemantics(SuffixOf)
// DefineSemantics(Flatten)
DefineSemantics(BNode)
DefineSemantics(BSize)
DefineSemantics(BLeaf)
DefineSemantics(BFold)
DefineSemantics(BIContent)
DefineSemantics(BLContent)
DefineSemantics(BContent)
DefineSemantics(BLeft)
DefineSemantics(BRight)
DefineSemantics(BIsLeaf)
DefineSemantics(BMatch)
DefineSemantics(BMoveL)
DefineSemantics(BMoveR)
DefineSemantics(BAccess)
DefineSemantics(Reverse)
DefineSemantics(BIdTree)
DefineSemantics(Int)
DefineSemantics(Lq)
DefineSemantics(Not)
DefineSemantics(Append)
DefineSemantics(Pow)


namespace semantics{
    Semantics* string2Semantics(const std::string& name);
}

#endif //DPSYNTHESISNEW_SEMANTICS_H
