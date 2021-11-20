//
// Created by pro on 2021/8/27.
//

#ifndef DPSYNTHESISNEW_PROGRAM_H
#define DPSYNTHESISNEW_PROGRAM_H

#include "semantics.h"
#include "complex_value.h"

class Program {
public:
    class::Type* oup_type;
    int ref = 1;
    void addRef();
    void delRef();
    Data run(const DataList& inp) const;
    Data run(const DataList& inp, const DataList& env) const;
    CollectRes collect(const DataList& inp) const;
    CollectRes collect(const DataList& inp, const DataList& env) const;
    virtual Data run(ExecuteInfo& info) const = 0;
    Program(class::Type* _oup_type): oup_type(_oup_type) {}
    virtual std::string toString() const = 0;
    virtual std::vector<class::Program*> getSubPrograms() const = 0;
    virtual class::Program* clone(const std::vector<class::Program*>& sub_list) = 0;
    int size() const;
    class::Program* copy();
    virtual ~Program() = default;
};

typedef std::vector<class::Program*> ProgramList;
typedef std::vector<ProgramList> ProgramStorage;

class SemanticsProgram: public Program {
public:
    class::Semantics* semantics;
    std::vector<Program*> sub_list;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return sub_list;}
    virtual class::Program* clone(const ProgramList& sub_list);
    SemanticsProgram(class::Semantics* _semantics, const std::vector<Program*>& _sub_list);
    virtual ~SemanticsProgram();
};

typedef std::vector<std::pair<std::string, class::Type*>> ParamList;

class LambdaProgram: public Program {
public:
    ParamList param_list;
    Program* content;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return {content};}
    virtual class::Program* clone(const ProgramList& sub_list);
    LambdaProgram(const ParamList& _param_list, Program* content);
    virtual ~LambdaProgram();
};

class LetProgram: public Program {
public:
    std::string var_name;
    Program *def, *content;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return {def, content};}
    virtual class::Program* clone(const ProgramList& sub_list);
    LetProgram(const std::string& _var_name, Program* _def, Program* _content);
    virtual ~LetProgram();
};

class ForEachProgram: public Program {
public:
    std::string var_name;
    Program *range, *content;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return {range, content};}
    virtual class::Program* clone(const ProgramList& sub_list);
    ForEachProgram(const std::string& _var_name, Program* _range, Program* _content);
    virtual ~ForEachProgram();
};

class IfProgram: public Program {
public:
    Program *cond, *tb, *fb;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {
        if (fb) return {cond, tb, fb};
        return {cond, tb};
    }
    virtual class::Program* clone(const ProgramList& sub_list);
    IfProgram(Program* _cond, Program* _tb, Program* _fb = nullptr);
    virtual ~IfProgram();
};

class SemicolonProgram: public Program {
public:
    std::vector<Program*> stmt_list;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return {stmt_list};}
    SemicolonProgram(const std::vector<Program*>& _stmt_list);
    virtual class::Program* clone(const ProgramList& sub_list);
    virtual ~SemicolonProgram();
};

class EmptyProgram: public Program {
public:
    virtual Data run(ExecuteInfo& info) const {return Data();}
    virtual std::string toString() const {return "Skip";}
    virtual std::vector<class::Program*> getSubPrograms() const {return {};}
    EmptyProgram(): Program(TVOID) {}
    virtual class::Program* clone(const ProgramList& sub_list);
    virtual ~EmptyProgram() = default;
};

class ApplyProgram: public Program {
public:
    Program* f;
    ProgramList param;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {
        ProgramList res = {f};
        for (auto* sub: param) res.push_back(sub);
        return res;
    }
    ApplyProgram(Program* _f, const ProgramList& _param_list);
    virtual class::Program* clone(const ProgramList& sub_list);
    virtual ~ApplyProgram();
};

class ProdProgram: public Program {
public:
    ProgramList param;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return param;}
    ProdProgram(const ProgramList& _param);
    virtual class::Program* clone(const ProgramList& sub_list);
    virtual ~ProdProgram();
};

class AccessProgram: public Program {
public:
    Program* s;
    int ind;
    virtual Data run(ExecuteInfo& info) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return {s};}
    AccessProgram(Program* _s, int _ind);
    virtual class::Program* clone(const ProgramList& sub_list);
    virtual ~AccessProgram();
};

enum CmpType {
    EQ, LEQ, GEQ
};

class CmpProgram: public Program {
public:
    Program* key;
    CmpType type;
    virtual Data run(ExecuteInfo& info) const;
    Data run(const DataList& x, const DataList& y, const DataList& env) const;
    Data getValue(const DataList& x, const DataList& env) const;
    virtual std::string toString() const;
    virtual std::vector<class::Program*> getSubPrograms() const {return {key};}
    CmpProgram(CmpType _type, Program* _key);
    virtual class::Program* clone(const ProgramList& sub_list);
    bool isEq() {return type == EQ;}
    virtual ~CmpProgram();
};

namespace cmp {
    std::string cmp2String(CmpType type);
}

namespace data {
    std::string collectRes2String(const CollectRes& res);
}

namespace program {
    class::Program* buildAnonymousProgram(AnonymousSemantics* semantics);
    class::Program* removeAllLet(class::Program* program);
    class::Program* rewriteParams(class::Program* program, const ProgramList& param_list);
    class::Program* rewriteProgramWithMap(class::Program* program, const std::map<std::string, class::Program*>& map);
    class::Program* buildParam(int id, class::Type* type);
    class::Program* buildTmp(const std::string& name, class::Type* type);
    class::Program* buildCollect(int id, class::Program* content);
    class::Program* buildConst(const Data& data);
    class::Program* rewriteCollect(class::Program* p, class::Program* x, class::Program* y);
    class::Program* removeProdAccess(class::Program* p);
    class::Type* extractCollectType(class::Program* p);

    struct ComponentInfo {
        class::Type* type;
        class::Program* program;
        std::vector<int> trace;
        ComponentInfo(class::Type* _type, class::Program* _program, const std::vector<int>& _trace);
        ComponentInfo() = default;
    };
    std::vector<ComponentInfo> extractAllComponents(class::Type* type, class::Program* p);
    ProgramList extractUsedTmps(class::Program* p);
    std::pair<int, class::Program*> unfoldCollect(class::Program* p);
    bool isConstant(class::Program* p);
    bool isParam(class::Program* p);
    bool applyCmp(CmpType type, const Data& x, const Data& y);
    std::string programList2String(const ProgramList& program_list);
    std::vector<int> getIntConsts(class::Program* program);
    bool isRef(class::Program* p);
    bool isNil(class::Program* p);
}

#endif //DPSYNTHESISNEW_PROGRAM_H
