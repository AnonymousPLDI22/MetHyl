//
// Created by pro on 2021/1/20.
//

#ifndef CPP_ENUMERATE_BASED_SOLVER_H
#define CPP_ENUMERATE_BASED_SOLVER_H

#include "bitset.h"
#include "task.h"
#include "time_guard.h"
#include <queue>
#include <unordered_set>

typedef unsigned long long HASH;

namespace autolifter {
    struct EnumerateInfo {
        int l_id, r_id;
        HASH h;
        std::vector<int> ind_list;
        Bitset info;

        EnumerateInfo(int _l_id, int _r_id, HASH _h, const std::vector<int> &_ind_list, const Bitset &_info) :
                l_id(_l_id), r_id(_r_id), h(_h), ind_list(_ind_list), info(_info) {
        }
    };

    class MaximalInfoList {
    public:
        std::vector<EnumerateInfo *> info_list;
        int num;

        void clear() { num = 0; }

        MaximalInfoList() : info_list(100), num(0) {}

        bool isAdd(EnumerateInfo *x);

        int add(EnumerateInfo *x);
    };

    class OccamVerifier {
    public:
        Task *task;
        std::unordered_map<std::string, std::pair<int, Data>> data_map;
        int start_verify_pos = 0;
        int verified_example_num = 0;
        int example_num;

        OccamVerifier(Task *_task) : task(_task), example_num(_task->config.KInitExampleNum) {}

        std::pair<int, int> verify(const std::vector<Program *> &program_list);

    };

    class SfSolver {
        void getMoreComponents();

        void addCounterExample(std::pair<int, int> counter_example);

        void addValidExample(int id);

        void initNewProgram();

        bool isAddExpression(int k, EnumerateInfo *info);

        std::pair<EnumerateInfo *, EnumerateInfo *> addExpression(int k, EnumerateInfo *info);

        HASH getComponentHash(int k);

        std::vector<Program *> getProgramList(EnumerateInfo *info);

        std::vector<Program *> synthesisFromExample();

        std::pair<EnumerateInfo *, EnumerateInfo *> getNextComposition(int k);

        std::pair<EnumerateInfo *, EnumerateInfo *> recoverResult(EnumerateInfo *info);

        bool synthesisFromExisting(const ProgramList& existing_list, ProgramList& tmp);

    public:
        std::vector<MaximalInfoList> maximal_list;
        MaximalInfoList global;
        std::vector<Bitset> program_info;
        std::vector<std::pair<int, int>> last;
        std::vector<Program *> program_space;
        std::vector<bool> is_valid;
        std::vector<int> valid_examples;
        std::vector<std::vector<EnumerateInfo *>> info_list;
        std::vector<std::pair<int, int> > example_list;
        std::unordered_set<unsigned long long> maximal_set;
        std::vector<HASH> hash_pool;
        int next_component_id = 0;
        Task *task;
        OccamVerifier *v;

        std::vector<Program *> synthesis(const std::vector<Program*>& existing_list);
        TimeGuard *guard;
        SfSolver(Task *_task, TimeGuard* _guard);

        ~SfSolver();
    };
}


#endif //CPP_ENUMERATE_BASED_SOLVER_H
