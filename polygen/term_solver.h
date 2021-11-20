//
// Created by pro on 2021/5/5.
//

#ifndef CPP_TERM_SOLVER_H
#define CPP_TERM_SOLVER_H

#include "solver/grammar.h"
#include "program.h"
#include "task.h"
#include "example_space.h"
#include "polygen_config.h"
#include "time_guard.h"

namespace polygen {
    class TermSolver {
        PolyGenConfig c;

        std::vector<PointExample *>
        buildExtendedExamples(const std::vector<Program *> &atom_list, const std::vector<PointExample *> &example_list);

        std::vector<Program *> getTerms(const std::vector<PointExample *> &example_list, int N, int K, TimeGuard* guard);

    public:

        void clearCache();

        std::vector<Program *> getTerms(const std::vector<PointExample *> &example_list, TimeGuard* guard = nullptr);

        TermSolver(const PolyGenConfig& _c): c(_c) {}
    };
}


#endif //CPP_TERM_SOLVER_H
