//
// Created by pro on 2021/10/27.
//

#include "state_mapping_solver.h"
#include <algorithm>

std::string NeqExample::toString() const {
    return data::dataList2String(state_x) + "!=" + data::dataList2String(state_y) + "@" + data::dataList2String(env);
}

bool NeqExample::isValid(Program *key) const {
    try {
        return key->run(state_x, env) != key->run(state_y, env);
    } catch (SemanticsError& e){
        return false;
    }
}

NeqExample::NeqExample(const DataList &_state_x, const DataList &_state_y, const DataList &_env):
        state_x(_state_x), state_y(_state_y), env(_env) {
}

StateExecutionLog::StateExecutionLog(ExecutionLog *log): env(log->env) {
    env.push_back(log->start->state);
    auto state_feature = [](State* state) {
        std::vector<std::string> plan_features;
        for (auto& p: state->plan_list) {
            plan_features.push_back(p.toString());
        }
        std::sort(plan_features.begin(), plan_features.end());
        std::string feature;
        for (auto& s: plan_features) feature += "@" + s;
        return feature;
    };
    std::unordered_map<std::string, int> tag_map;
    int tag_num = 0;
    for (auto* state: log->state_list) {
        state_list.push_back(data::unfoldProdData(state->state));
        auto feature = state_feature(state);
        if (tag_map.find(feature) == tag_map.end()) {
            tag_map[feature] = tag_num;
            tag_list.push_back(tag_num);
            ++tag_num;
        } else tag_list.push_back(tag_map[feature]);
    }
}

DataList * StateExecutionLog::getOutputs(Program *p) {
    auto feature = p->toString();
    if (oup_cache.find(feature) != oup_cache.end()) return oup_cache[feature];
    auto* res = new DataList();
    for (auto& state: state_list) res->push_back(p->run(state, env));
    oup_cache[feature] = res;
    return res;
}

std::vector<std::pair<int, int> > StateExecutionLog::extractExamples(KeyEqRelation *relation, int num_limit) {
    std::vector<DataList*> oup_list;
    for (auto* key: relation->key_list) {
        oup_list.push_back(getOutputs(key));
    }
    std::vector<int> state_id(state_list.size());
    for (int i = 0; i < state_list.size(); ++i) state_id[i] = i;
    std::random_shuffle(state_id.begin(), state_id.end());
    std::vector<std::pair<int, int>> res;
    std::unordered_map<std::string, std::vector<int> > feature_map;
    auto state_feature = [=](int id) {
        std::string feature;
        for (auto* oups: oup_list) feature += std::to_string(oups->at(id).getInt()) + "@";
        return feature;
    };
    for (int id: state_id) {
        auto feature = state_feature(id);
        auto& list = feature_map[feature];
        for (int pre: list) {
            if (tag_list[id] != tag_list[pre]) {
                res.emplace_back(id, pre);
                if (res.size() == num_limit) return res;
            }
        }
        list.push_back(id);
    }
    return res;
}

bool StateExecutionLog::verify(KeyEqRelation *relation) {
    return extractExamples(relation, 1).empty();
}

int StateExecutionLog::getCoveredNum(const std::vector<std::pair<int, int>> &example_list, Program *p) {
    auto* oup_list = getOutputs(p);
    int num = 0;
    for (const auto& example: example_list) {
        if (oup_list->at(example.first) != oup_list->at(example.second)) {
            ++num;
        }
    }
    return num;
}

NeqExample StateExecutionLog::buildExample(const std::pair<int, int> &local_example) {
    return {state_list[local_example.first], state_list[local_example.second], env};
}

StateExecutionLog::~StateExecutionLog() {
    for (auto info: oup_cache) {
        delete info.second;
    }
}

std::string StateExecutionLog::getFeature(Program *p) {
    auto name = p->toString();
    if (feature_map.count(name)) return feature_map[name];
    auto* oups = getOutputs(p);
    std::unordered_map<std::string, int> id_map;
    std::string feature;
    for (int i = 0; i < state_list.size(); ++i) {
        auto oup = oups->at(i).toString();
        if (id_map.count(oup) == 0) id_map[oup] = i;
        feature += "," + std::to_string(id_map[oup]);
    }
    return feature_map[name] = feature;
}