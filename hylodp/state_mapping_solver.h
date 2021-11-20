//
// Created by pro on 2021/10/20.
//

#ifndef DPSYNTHESISNEW_STATE_MAPPING_SOLVER_H
#define DPSYNTHESISNEW_STATE_MAPPING_SOLVER_H

#include "input_sampler.h"
#include "preorder.h"
#include "solver/cegis.h"

class NeqExample: public CEGISExample {
public:
    DataList state_x, state_y;
    DataList env;
    virtual bool isValid(Program* key) const;
    std::string toString() const;
    NeqExample(const DataList& _state_x, const DataList& _state_y, const DataList& _env);
};

class StateExecutionLog {
public:
    DataList env;
    DataStorage state_list;
    std::vector<int> tag_list;

    std::unordered_map<std::string, DataList*> oup_cache;
    DataList* getOutputs(Program* p);
    StateExecutionLog(ExecutionLog* log);
    std::vector<std::pair<int, int> > extractExamples(KeyEqRelation* order, int num_limit);
    bool verify(KeyEqRelation* order);
    int getCoveredNum(const std::vector<std::pair<int, int>>& example_list, Program* p);
    NeqExample buildExample(const std::pair<int, int>& local_example);
    ~StateExecutionLog();

    std::unordered_map<std::string, std::string> feature_map;
    std::string getFeature(Program* p);
};

struct StateSeedInfo {
    int pos;
    std::pair<int, int> local_example;
    std::unordered_set<int> pos_set;
    StateSeedInfo(int _pos, std::pair<int, int> _example, std::unordered_set<int>&& _set):
        pos(_pos), local_example(_example), pos_set(_set) {}
};

class StateMappingSolver {
    std::vector<StateExecutionLog*> log_list;
    std::unordered_map<std::string, int> range_cache;
    ProgramStorage program_storage;
    int evaluateRange(Program* p);
    ProgramList getComponentList(int enumerate_num);
    StateExecutionLog* verifyRelation(KeyEqRelation* relation);
    KeyEqRelation* synthesis(int num, int enumerate_num);
    int getCost(KeyEqRelation* relation);
    bool runFullExample(const std::pair<int, std::pair<int, int>>& full_example, Program* p);
    KeyEqRelation* search(int num, ProgramList& component_space, KeyEqRelation* relation, std::vector<int> seeds, int ans);
    std::string getFeature(Program* p);

    std::vector<StateSeedInfo> seed_info_list;
    std::unordered_set<int> full_set;
    void prepareSeedInfo(const ProgramList& program_list, const std::vector<std::pair<int, std::pair<int ,int>>>& example_space);
    std::vector<int> getCandidateAccordingToSeeds(int num, std::vector<int> seeds);
    std::vector<int> getRemainSeeds(const std::vector<int>& seeds, int pos);
    void removeDuplicated(int pos);
    TimeGuard* guard;

public:
    InputSampler *sampler, *small_sampler;
    Task* task;
    StateMappingSolver(InputSampler* _sampler, int timeout = -1);
    KeyEqRelation* synthesisEqRelation();
    ~StateMappingSolver();
};


#endif //DPSYNTHESISNEW_STATE_MAPPING_SOLVER_H
