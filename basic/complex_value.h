//
// Created by pro on 2021/9/1.
//

#ifndef DPSYNTHESISNEW_COMPLEX_VALUE_H
#define DPSYNTHESISNEW_COMPLEX_VALUE_H

#include "data.h"

class ListValue: public Value {
public:
    DataList value;
    ListValue(const DataList& _value, class::Type* _type=nullptr);
    virtual std::string toString() const;
    virtual bool equal(Value* v) const;
    int size() const {return value.size();}
protected:
    virtual ~ListValue() = default;
};

class BTreeValue: public Value {
public:
    Data l, r, v;
    BTreeValue(const Data& _v, const Data& _l, const Data& _r, class::Type* _type=nullptr);
    virtual std::string toString() const;
    virtual bool equal(Value* v) const;
    int size() const;
    bool isLeaf() const;
};

class ProdValue: public Value {
public:
    std::vector<Data> value;
    ProdValue(const DataList& _value, class::Type* _type=nullptr);
    virtual std::string toString() const;
    virtual bool equal(Value* v) const;
protected:
    virtual ~ProdValue() = default;
};

class Semantics;

class ArrowValue: public Value {
public:
    class::Semantics* semantics;
    bool is_anonymous;
    ArrowValue(class::Semantics* _semantics, bool _is_anonymous);
    virtual ~ArrowValue();
    virtual std::string toString() const;
    virtual bool equal(Value* _value) const {return false;}
};


#endif //DPSYNTHESISNEW_COMPLEX_VALUE_H
