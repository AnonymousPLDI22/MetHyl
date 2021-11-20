//
// Created by pro on 2021/9/14.
//

#ifndef DPSYNTHESISNEW_SOLVER_H
#define DPSYNTHESISNEW_SOLVER_H

#include "task.h"

class Solver {
public:
    Task* task;
    Solver(Task* _task): task(_task) {}
};


#endif //DPSYNTHESISNEW_SOLVER_H
