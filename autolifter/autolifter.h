//
// Created by pro on 2020/12/2.
//

#ifndef CPP_COMPLETE_SOLVER_H
#define CPP_COMPLETE_SOLVER_H

#include "sf_solver.h"
#include "sc_solver.h"

namespace autolifter {
    struct TmpCombinatorInfo {
        std::vector<int> inp_list;
        Program *program;

        TmpCombinatorInfo(const std::vector<int> &_inp_list, Program *prog) : inp_list(_inp_list), program(prog) {}
    };

    class AutoLifter {
        Program *rewriteProgram(const TmpCombinatorInfo &info, Task *task, Type* x_type);
    public:
        std::vector<Task *> task_list;
        std::vector<Program *> lifting_list;
        std::vector<std::vector<Program *>> combinator_list;

        int getLiftingId(Program *program);
        TimeGuard* guard;

        AutoLifter(const std::vector<Task *> &_task_list, const std::vector<Program *> &init_programs, TimeGuard* _guard);

        void synthesis();
        void printResult() const;
    };
}

#endif //CPP_COMPLETE_SOLVER_H
