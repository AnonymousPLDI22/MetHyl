//
// Created by pro on 2021/10/20.
//

#include "example_space.h"

BatchedExampleSpace::BatchedExampleSpace(const std::vector<ExampleSpace *> &_space_list):
        client_space_list(_space_list), id_list(_space_list.size()) {
    for (int i = 0; i < id_list.size(); ++i) id_list[i] = 0;
    for (auto* sub_space: client_space_list) sub_space->addUsage();
}

void BatchedExampleSpace::acquireMoreExamples() {
    int num = rand() % (id_list.size());
    auto* sub_example = client_space_list[num]->getExample(id_list[num]);
    example_space.push_back(new Example(*sub_example));
    id_list[num] += 1;
}

BatchedExampleSpace::~BatchedExampleSpace() noexcept {
    for (auto* sub: client_space_list) sub->delUsage();
}

std::string example::pointExample2String(const PointExample &example) {
    return data::dataList2String(example.first) + "->" + example.second.toString();
}