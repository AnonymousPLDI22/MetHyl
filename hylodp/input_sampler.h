//
// Created by pro on 2021/9/14.
//

#ifndef DPSYNTHESISNEW_INPUT_SAMPLER_H
#define DPSYNTHESISNEW_INPUT_SAMPLER_H

#include "sampler.h"
#include "executor.h"
#include "config.h"

typedef std::vector<double> AlphaList;
typedef std::vector<AlphaList> AlphaStorage;
typedef std::pair<Data, DataList> TaskInp;

class SamplerLimitConfig {
public:
    int size_limit = config::KINF;
    int in_int_max = 3;
    int in_int_min = -3;
    int out_int_max = 10;
    int out_int_min = -10;
    Type* getLimitedType(Type* t, bool is_out) const;
};

class InputSampler {
    void setSampleType(const AlphaList& state_alpha, const AlphaStorage& env_alpha);
    bool isValidAlpha(const AlphaList& state_alpha, const AlphaStorage& env_alpha);
    InputSampler(Task* _t, Sampler *_s, Type* sample_state_type, const TypeList& sample_env_type);
public:
    Sampler* s;
    Task* t;
    Executor* e;
    Type* sample_state_type;
    TypeList sample_env_type;
    InputSampler(Task* _t, Executor* _e = nullptr, Sampler* _s=nullptr);
    TaskInp sampleTaskExample() const;
    void printType(FILE* file) const;
    InputSampler* getLimitedSampler(const SamplerLimitConfig& c = {});
    ExecutionLog* getLog(double time_out = 1.0) const;
};


#endif //DPSYNTHESISNEW_INPUT_SAMPLER_H
