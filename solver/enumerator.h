//
// Created by pro on 2021/9/7.
//

#ifndef DPSYNTHESISNEW_ENUMERATOR_H
#define DPSYNTHESISNEW_ENUMERATOR_H

#include "program.h"
#include "time_guard.h"
#include "solver/grammar.h"
#include "config.h"
#include <unordered_set>

class Verifier {
public:
    virtual bool isValid(Program* program) = 0;
    virtual ~Verifier() = default;
};

class AllCollectVerifier: public Verifier {
public:
    virtual bool isValid(Program* program) {return true;}
    virtual ~AllCollectVerifier() = default;
};

class PointVerifier: public Verifier {
public:
    std::vector<PointExample*> example_list;
    PointVerifier(const std::vector<PointExample*>& _e): example_list(_e) {}
    virtual bool isValid(Program *program);
    ~PointVerifier() = default;
};

class Optimizer {
public:
    virtual bool isDuplicated(int symbol_id, Program* program) = 0;
    virtual bool isAccept(Program* program) = 0;
    virtual ~Optimizer() = default;
    virtual void clear() = 0;
};

class DefaultOptimizer: public Optimizer {
public:
    TimeGuard* guard = nullptr;
    DefaultOptimizer(TimeGuard* _guard = nullptr): guard(_guard) {}
    virtual bool isDuplicated(int symbol_id, Program* program) {
        if (guard && guard->isTimeOut()) throw TimeOutError();
        return false;
    }
    virtual bool isAccept(Program* program) {return true;}
    virtual ~DefaultOptimizer() = default;
    virtual void clear() {}
};

class ValidOptimizer: public Optimizer {
public:
    DataStorage storage;
    ValidOptimizer(const DataStorage& _s): storage(_s) {}
    virtual bool isDuplicated(int symbol_id, Program* program);
    virtual bool isAccept(Program* program) {return true;}
    virtual ~ValidOptimizer() = default;
    virtual void clear() {};
};

class OEOptimizer: public Optimizer {
public:
    DataStorage test_inp_list;
    int int_limit;
    TimeGuard* guard;
    std::unordered_map<std::string, std::string> feature_set;
    virtual bool isDuplicated(int symbol_id, Program* program);
    virtual bool isAccept(Program* program);
    OEOptimizer(DataStorage&& _test_inp_list, int _int_limit = config::KINF, TimeGuard* _guard = nullptr):
        test_inp_list(_test_inp_list), int_limit(_int_limit), guard(_guard) {}
    virtual ~OEOptimizer() = default;
    virtual void clear();
};

class EnumConfig {
public:
    int num_limit, size_limit;
    int calc_num = 1000000000;
    Verifier* v;
    Optimizer* o;
    EnumConfig(Verifier* _v, Optimizer* _o, int _num_limit = 1, int _size_limit = 1000000);
};

typedef std::pair<DataList, Data> PointExample;
typedef std::vector<PointExample> PointExampleList;

namespace enumerate {
    std::vector<Program*> synthesis(Grammar* g, const EnumConfig& c);
}

#endif //DPSYNTHESISNEW_ENUMERATOR_H
