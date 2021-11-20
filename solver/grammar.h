//
// Created by pro on 2021/9/2.
//

#ifndef DPSYNTHESISNEW_GRAMMAR_H
#define DPSYNTHESISNEW_GRAMMAR_H

#include "program.h"

class Rule;

class NonTerminal {
public:
    std::string name;
    Type* type;
    int id = 0;
    std::vector<Rule*> rule_list;
    NonTerminal(const std::string& _name, Type* _type);
    std::string toString() const;
};

typedef std::vector<NonTerminal*> SymbolList;

class Rule {
public:
    SymbolList param_list;
    virtual Program* buildProgram(const ProgramList& sub_list) = 0;
    virtual std::string toString() const = 0;
    Rule(const SymbolList& _param_list);
};

class SemanticsRule: public Rule {
public:
    Semantics* semantics;
    SemanticsRule(Semantics* _semantics, const SymbolList& _param_list);
    virtual SemanticsProgram* buildProgram(const ProgramList& sub_list);
    virtual std::string toString() const;
};

class LambdaRule: public Rule {
public:
    ParamList inp_list;
    LambdaRule(const ParamList& _inp_list, NonTerminal* content);
    virtual LambdaProgram* buildProgram(const ProgramList& sub_list);
    virtual std::string toString() const;
};

class AccessRule: public Rule {
public:
    NonTerminal* prod;
    int id;
    virtual AccessProgram* buildProgram(const ProgramList& sub_list);
    virtual std::string toString() const;
    AccessRule(NonTerminal* _prod, int _id);
};

class CmpRule: public Rule {
public:
    NonTerminal* key;
    CmpType type;
    virtual CmpProgram* buildProgram(const ProgramList& sub_list);
    virtual std::string toString() const;
    CmpRule(CmpType _type, NonTerminal* _key);
};

class ExtraProgramRule: public Rule {
public:
    Program* p;
    virtual Program* buildProgram(const ProgramList& sub_list);
    virtual std::string toString() const;
    ExtraProgramRule(Program* _p);
};

class Grammar {
public:
    NonTerminal* start_symbol;
    SymbolList symbol_list;
    Grammar(NonTerminal* _start_symbol, const SymbolList& _symbol_list);
    void indexSymbols() const;
    void print() const;
};

struct GrammarConfig {
public:
    int lambda_depth = 1, tmp_num = 3;
    Type* oup_type = TINT;
    TypeList basic_types;
    TypeList var_list;
    DataList const_list;
    std::vector<std::string> op_list;
};

namespace grammar {
    Grammar* buildGrammar(const GrammarConfig& config);
    Grammar* buildDefaultGrammar(const TypeList& var_list, Type* oup_type);
    NonTerminal* getDefaultSymbolForType(Type* type);
}


#endif //DPSYNTHESISNEW_GRAMMAR_H
