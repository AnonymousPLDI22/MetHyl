//
// Created by pro on 2021/10/25.
//

#ifndef DPSYNTHESISNEW_HYLODP_H
#define DPSYNTHESISNEW_HYLODP_H

#include "task.h"
#include "executor.h"

struct HyloDPRes {
    Task* task;
    Executor* e;
};

namespace hylodp{
    extern HyloDPRes synthesisDP(Task* task, const std::string& oup_file = "");
}

#endif //DPSYNTHESISNEW_HYLODP_H
