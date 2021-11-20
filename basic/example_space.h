//
// Created by pro on 2021/10/20.
//

#ifndef DPSYNTHESISNEW_EXAMPLE_SPACE_H
#define DPSYNTHESISNEW_EXAMPLE_SPACE_H

#include "data.h"
#include "program.h"
#include <map>

template<class TExample>
class TemplateExampleSpace {
public:
    int use_num = 0;
    std::vector<TExample *> example_space;

    virtual void acquireMoreExamples() = 0;

    TExample *getExample(int k) {
        while (example_space.size() <= k) acquireMoreExamples();
        return example_space[k];
    }

    void addUsage() {use_num += 1;}
    void delUsage() {
        use_num -= 1;
        if (!use_num) delete this;
    }

    virtual ~TemplateExampleSpace() {
        for (auto* example: example_space) delete example;
    }
};

typedef TemplateExampleSpace<Example> ExampleSpace;
typedef TemplateExampleSpace<PointExample> PointExampleSpace;

class BatchedExampleSpace: public ExampleSpace {
public:
    std::vector<ExampleSpace*> client_space_list;
    std::vector<int> id_list;
    BatchedExampleSpace(const std::vector<ExampleSpace*>& _space_list);
    virtual void acquireMoreExamples();
    virtual ~BatchedExampleSpace();
};

namespace example{
    std::string pointExample2String(const PointExample& example);
}



#endif //DPSYNTHESISNEW_EXAMPLE_SPACE_H
