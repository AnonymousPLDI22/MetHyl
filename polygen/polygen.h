//
// Created by pro on 2021/5/5.
//

#ifndef CPP_POLYGEN_H
#define CPP_POLYGEN_H

#include "term_solver.h"
#include "unifier.h"
#include "solver/grammar.h"
#include "example_space.h"
#include "time_guard.h"

namespace polygen {
    class PolyGen {
        TermSolver *term_solver;
        Unifier *unifier;
        PolyGenConfig c;
    public:
        PolyGen(const PolyGenConfig& _c) : term_solver(new TermSolver(_c)), unifier(new Unifier(_c)), c(_c) {
        }

        void clearCache() {
            term_solver->clearCache();
        }

        Program *synthesis(const std::vector<PointExample *> &example_list, TimeGuard* guard = nullptr);
        Program* synthesis(PointExampleSpace* example_space, TimeGuard* guard = nullptr);
        Program* refSynthesis(PointExampleSpace* example_space, TimeGuard* guard = nullptr);
    };

    extern PolyGen* buildDefaultPolyGen(const std::vector<int>& int_const = {0});
}


#endif //CPP_POLYGEN_H
