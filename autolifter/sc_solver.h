//
// Created by pro on 2021/5/5.
//

#ifndef CPP_EXTERNAL_SOLVER_H
#define CPP_EXTERNAL_SOLVER_H

#include "program.h"
#include "example_space.h"
#include "task.h"
#include "time_guard.h"
#include "polygen/polygen.h"
#include <unordered_set>

namespace autolifter {
    class ScSolver {
        void insertExample(PointExample &&example);

    public:
        std::vector<PointExample *> io_example_space;
        std::unordered_set<std::string> example_set;
        std::vector<Program *> lifting_list;
        Program* enumSynthesis(bool is_force=false);
        int example_pos = 0;
        Task *task;
        polygen::PolyGen *client_solver;
        TimeGuard * guard;

        ScSolver(Task *_task, const std::vector<Program *> &_lifting_list, TimeGuard* _guard);

        void acquireExample(int limit);

        Program *synthesis();

        ~ScSolver() {
            delete client_solver;
            for (auto *example: io_example_space) delete example;
        }
    };
}

#endif //CPP_EXTERNAL_SOLVER_H
