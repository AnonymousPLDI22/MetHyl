//
// Created by pro on 2021/9/14.
//

#ifndef DPSYNTHESISNEW_SAMPLER_H
#define DPSYNTHESISNEW_SAMPLER_H

#include "task.h"
#include <random>

class Sampler {
public:
    virtual Data sampleData(Type* type, const std::string& name) = 0;
};

class IntBasedSampler: public Sampler {
protected:
    virtual int getSize(int size_limit) = 0;
    virtual int getInt(int l, int r) = 0;
    virtual Data sampleTree(Type* type, int size);
public:
    virtual Data sampleData(Type* type, const std::string& name);
};

class UniformSampler: public IntBasedSampler {
protected:
    virtual int getSize(int size_limit);
    virtual int getInt(int l, int r);
public:
    std::default_random_engine e;
    UniformSampler(int seed = 0): e(seed) {}
};


#endif //DPSYNTHESISNEW_SAMPLER_H
