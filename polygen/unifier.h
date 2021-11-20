//
// Created by pro on 2021/5/5.
//

#ifndef CPP_UNIFIER_H
#define CPP_UNIFIER_H

#include "program.h"
#include "bitset.h"
#include "task.h"
#include "polygen_config.h"
#include "example_space.h"
#include "time_guard.h"

namespace polygen {

    struct CmpInfo {
        Program *cmp;
        Bitset P, N;

        CmpInfo(Program *_cmp, const Bitset &_P, const Bitset &_N) : cmp(_cmp), P(_P), N(_N) {}
    };

    class Unifier {
        std::vector<CmpInfo *> excludeDuplicated(const std::vector<Program *> &program_list);

        bool verifySolvable(const std::vector<CmpInfo *> &cmp_list);

        PolyGenConfig c;
    public:
        std::vector<Program *> extra_list;
        std::vector<int> param_list;
        std::vector<PointExample *> P, N;

        Program *getCondition(const std::vector<PointExample *> &positive_example,
                              const std::vector<PointExample *> &negative_example,
                              TimeGuard* guard = nullptr);

        Unifier(PolyGenConfig _c) : c(_c) {}
    };
}


#endif //CPP_UNIFIER_H
