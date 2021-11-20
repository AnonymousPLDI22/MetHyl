//
// Created by pro on 2021/10/11.
//

#ifndef DPSYNTHESISNEW_POLYGEN_CONFIG_H
#define DPSYNTHESISNEW_POLYGEN_CONFIG_H

#include "solver/grammar.h"

namespace polygen {
    class PolyGenConfig {
    public:
        std::vector<std::string> extra_list;
        int KMaxTermNum = 2;
        int KMaxExampleNum = 5;
        int KTermIntMax = 2;
        int KRandomC = 5;
        int KInitExampleNum = 50;
        int KInitTermAtomNum = 20;
        int KInitPredicateNum = 10;
        int KTestExampleNum = 100;
        int KEnumeratorTimeout = 10000;
        int KAtomMaxNum = 100;
        bool isRef = false;
        std::vector<int> int_consts = {0, 1};
        void insertComponent(Program* p);
        void clear();
    };
}

#endif //DPSYNTHESISNEW_POLYGEN_CONFIG_H
