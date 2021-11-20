//
// Created by pro on 2021/9/15.
//

#ifndef DPSYNTHESISNEW_PLAN_ORDER_SOLVER_H
#define DPSYNTHESISNEW_PLAN_ORDER_SOLVER_H

#include "input_sampler.h"
#include "solver/cegis.h"

class NegCmpExample: public CEGISExample {
public:
    Data plan_x, plan_y;
    DataList env;
    virtual bool isValid(Program* cmp) const;
    std::string toString() const;
    NegCmpExample(const Data& _plan_x, const Data& _plan_y, const DataList& _env);
};

typedef std::pair<int, std::vector<int>> PlanEdge;
typedef std::vector<PlanEdge> PlanEdgeList;

class PlanExecutionLog {
public:
    DataList plan_list, env;
    std::vector<PlanEdgeList> edge_storage;
    std::map<std::string, DataList*> evaluate_cache;
    PlanExecutionLog(ExecutionLog* _log);
    bool verify(CmpPreOrder* order);
    std::vector<std::pair<int, int>> extractLocalExample(CmpPreOrder* order, int max_num);
    NegCmpExample buildExample(const std::pair<int, int>& local_example);
    // std::vector<NegCmpExample*> extractExample(CmpPreOrder* order);
    int getCoveredNum(CmpProgram* cmp, const std::vector<std::pair<int, int>>& example_list);
    std::unordered_map<std::string, std::string> feature_map;
    std::string getFeature(CmpProgram* cmp);
    void printEdge(const PlanEdge& edge);
    DataList* getOutputs(Program* p);
    ~PlanExecutionLog();
};

struct SeedExampleInfo {
    int pos;
    std::pair<int, int> example;
    std::vector<int> valid_programs;
};

class PlanOrderSolver {
    Grammar* buildDefaultGrammar(const TypeList& var_list, Type* oup_type);

    std::vector<PlanExecutionLog*> log_list;
    std::pair<int, int> getCost(CmpPreOrder* c);
    std::vector<SeedExampleInfo> seed_info_list;

    ProgramList getCandidateAccordingToSeeds(const ProgramList& program_list, int num, const std::vector<int>& remain_seeds);
    std::vector<int> filterSeed(const std::vector<int>& id_list, CmpProgram* cmp);
    void prepareSeedInfo(const ProgramList& component_list, const std::vector<std::pair<int, std::pair<int, int>>>& example_space);
    std::map<std::string, int> component_range_map;
    int evaluateRange(Program* p);

    ProgramList getComponentList(int enumerate_num);
    CmpPreOrder* search(int component_num, const ProgramList& program_list, CmpPreOrder* order, std::vector<int> seeds, int lim);
    CmpPreOrder* synthesis(int component_num, int enumerate_num);
    PlanExecutionLog* verifyOrder(CmpPreOrder* order);
    TimeGuard * guard = nullptr;
    std::string getFeature(CmpProgram* p);
public:
    InputSampler* sampler;
    Task* task;
    PlanOrderSolver(InputSampler* _s, int time_out = -1);
    CmpPreOrder* synthesisPreOrder();
    ~PlanOrderSolver();
};


#endif //DPSYNTHESISNEW_PLAN_ORDER_SOLVER_H
