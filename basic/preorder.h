//
// Created by pro on 2021/9/14.
//

#ifndef DPSYNTHESISNEW_PREORDER_H
#define DPSYNTHESISNEW_PREORDER_H

#include "program.h"

typedef std::pair<CmpType, Program*> CmpUnit;

class PreOrder {
public:
    virtual int leq(const Data& x, const Data& y, const DataList& env) const = 0;
    virtual std::string toString() const = 0;
    void print() const;
    virtual DataList getMaximal(const DataList& data_list, const DataList& env) const;
    bool coveredBy(const DataList& x, const DataList& y, const DataList& env) const;
    virtual ~PreOrder() = default;
};

class EmptyPreOder: public PreOrder {
public:
    virtual int leq(const Data& x, const Data& y, const DataList& env) const {return 0;}
    virtual std::string toString() const {return "empty";}
    virtual DataList getMaximal(const DataList& data_list, const DataList& env) const {return data_list;}
};

class CmpPreOrder: public PreOrder {
public:
    std::vector<CmpProgram*> cmp_list;
    virtual int leq(const Data& x, const Data& y, const DataList& env) const;
    virtual DataList getMaximal(const DataList& data_list, const DataList& env) const;
    virtual std::string toString() const;
    CmpPreOrder(const std::vector<Program*>& _cmp_list);
    virtual ~CmpPreOrder() = default;
    CmpPreOrder* insert(CmpProgram* cmp) const;
};

class EqRelation: public PreOrder {
public:
    virtual int leq(const Data& x, const Data& y, const DataList& env) const;
    virtual DataList getMaximal(const DataList& data_list, const DataList& env) const;
    virtual std::string getFeature(const Data& x, const DataList& env) const = 0;
    virtual std::string toString() const = 0;
};

class KeyEqRelation: public EqRelation {
public:
    std::vector<Program*> key_list;
    bool is_unfold;
    virtual std::string toString() const;
    KeyEqRelation(const std::vector<Program*>& _cmp_list, bool _is_unfold = true);
    virtual std::string getFeature(const Data& x, const DataList& env) const;
    virtual ~KeyEqRelation() = default;
};

class IdRelation: public EqRelation {
public:
    virtual std::string getFeature(const Data& x, const DataList& env) const;
    virtual std::string toString() const {return "id";}
};


#endif //DPSYNTHESISNEW_PREORDER_H
