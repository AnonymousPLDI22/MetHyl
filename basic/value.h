//
// Created by pro on 2021/8/23.
//

#ifndef DPSYNTHESISNEW_VALUE_H
#define DPSYNTHESISNEW_VALUE_H

#include "type.h"
#include <iostream>

class Value {
protected:
    Type* type;
    int ref_counter;
public:
    Type* getType() const {return type;}
    Value(Type* _type): type(_type), ref_counter(0) {
    }
    void addRef() {ref_counter += 1;}
    void delRef() {
        ref_counter -= 1; if (ref_counter == 0) delete this;
    }
    virtual std::string toString() const = 0;
    virtual bool equal(Value* v) const = 0;
protected:
    virtual ~Value() {
    }
};

class IntValue: public Value {
    int value;
public:
    IntValue(int _value): Value(TINT), value(_value) {}
    virtual std::string toString() const {return std::to_string(value);}
    int getInt() const {return value;}
    virtual bool equal(Value* v) const;
protected:
    virtual ~IntValue() = default;
};

class BoolValue: public Value {
    bool value;
public:
    BoolValue(bool _value): Value(TBOOL), value(_value) {}
    virtual std::string toString() const {return std::to_string(value);}
    bool getBool() const {return value;}
    virtual bool equal(Value* v) const;
protected:
    virtual ~BoolValue() = default;
};

#endif //DPSYNTHESISNEW_VALUE_H
