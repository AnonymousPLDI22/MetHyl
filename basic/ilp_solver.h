//
// Created by pro on 2021/10/20.
//

#ifndef DPSYNTHESISNEW_ILP_SOLVER_H
#define DPSYNTHESISNEW_ILP_SOLVER_H

#include "example_space.h"

class ILPConfig {
public:
    int KConstMax = 10;
    int KCoefficientMax = 2;
};

namespace ilp{
    bool ILPFromExamples(const std::vector<PointExample*>& example_list, std::vector<int>& result, ILPConfig c = {});
    bool ILPFromExampleCEGIS(std::vector<PointExample*> example_list, std::vector<int>& result, ILPConfig c = {});
}


#endif //DPSYNTHESISNEW_ILP_SOLVER_H
