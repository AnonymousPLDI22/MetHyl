//
// Created by pro on 2021/10/27.
//

#include "plan_order_solver.h"
#include <algorithm>
#include <cassert>

NegCmpExample::NegCmpExample(const Data &_plan_x, const Data &_plan_y, const DataList &_env):
        plan_x(_plan_x), plan_y(_plan_y), env(_env) {}

bool NegCmpExample::isValid(Program* program) const {
    auto* cp = dynamic_cast<CmpProgram*>(program);
#ifdef DEBUG
    assert(cp);
#endif
    try {
        return !cp->run({plan_x}, {plan_y}, env).getBool();
    } catch (SemanticsError& e) {
        return false;
    }
}

std::string NegCmpExample::toString() const {
    return plan_x.toString() + "!<" + plan_y.toString() + "@" + data::dataList2String(env);
}

PlanExecutionLog::PlanExecutionLog(ExecutionLog *log): env(log->env) {
    std::unordered_map<std::string, int> plan_id_map;
    for (auto* state: log->state_list) {
        for (auto& plan: state->plan_list) {
            auto feature = plan.toString();
            if (plan_id_map.find(feature) == plan_id_map.end()) {
                plan_id_map[feature] = plan_list.size();
                plan_list.push_back(plan);
            }
        }
    }
    auto id = [plan_id_map](const Data& d)->int {
        return plan_id_map.find(d.toString())->second;
    };
    for (auto* state: log->state_list) {
        for (auto* trans: state->out_list) {
            std::vector<PlanEdge> edge_list;
            for (auto* exp: trans->exp_list) {
                int u = id(exp->plan);
                std::vector<int> v;
                for (auto& d: exp->plan_list) v.push_back(id(d));
                edge_list.emplace_back(u, v);
            }
            if (edge_list.size() <= 1) continue;
            // TODO: remove impossible trans
            edge_storage.push_back(edge_list);
        }
    }
}

DataList * PlanExecutionLog::getOutputs(Program *p) {
    auto feature = p->toString();
    if (evaluate_cache.count(feature)) return evaluate_cache[feature];
    auto* res = new DataList();
    for (auto& plan: plan_list) res->push_back(p->run({plan}, env));
    return evaluate_cache[feature] = res;
}

namespace {
    // TODO: to make it look better
    struct PlanCmpOrder {
        std::vector<DataList*> oups;
        std::vector<CmpType> cmp_types;
        PlanCmpOrder(CmpPreOrder* order, PlanExecutionLog* log) {
            for (auto* cmp: order->cmp_list) {
                cmp_types.push_back(cmp->type);
                oups.push_back(log->getOutputs(cmp->key));
            }
        }
        bool leq(int x, int y) const {
            for (int i = 0; i < oups.size(); ++i) {
                int wx = oups[i]->at(x).getInt();
                int wy = oups[i]->at(y).getInt();
                if (!program::applyCmp(cmp_types[i], wx, wy)) return false;
            }
            return true;
        }
        bool isCoveredBy(const std::vector<int>& x, const std::vector<int>& y) const {
            for (int wx: x) {
                bool is_covered = false;
                for (int wy: y) {
                    if (leq(wx, wy)) {
                        is_covered = true; break;
                    }
                }
                if (!is_covered) return false;
            }
            return true;
        }
    };
}

namespace {
    template<class T>
    std::vector<T> setToList(const std::set<T>& set) {
        std::vector<T> res;
        for (auto& w: set) res.push_back(w);
        std::random_shuffle(res.begin(), res.end());
        return res;
    }
}

void PlanExecutionLog::printEdge(const PlanEdge &edge) {
    Data x = plan_list[edge.first];
    DataList y; for (int w: edge.second) y.push_back(plan_list[w]);
    std::cout << x.toString() << "->" << data::dataList2String(y) << std::endl;
}

std::vector<std::pair<int, int>> PlanExecutionLog::extractLocalExample(CmpPreOrder *order, int max_num) {
    std::vector<DataList*> keys_list;
    for (auto* cmp: order->cmp_list) keys_list.push_back(getOutputs(cmp->key));
    PlanCmpOrder tmp_order(order, this);
    std::vector<int> edge_id_list;
    for (int i = 0; i < edge_storage.size(); ++i) edge_id_list.push_back(i);
    std::random_shuffle(edge_id_list.begin(), edge_id_list.end());
    std::set<std::pair<int, int>> res;
    for (auto edge_id: edge_id_list) {
        auto& edge_list = edge_storage[edge_id];
        for (int i = 0; i < edge_list.size(); ++i) {
            for (int j = 0; j < edge_list.size(); ++j) {
                if (i == j || res.find({edge_list[i].first, edge_list[j].first}) != res.end() ||
                    !tmp_order.leq(edge_list[i].first, edge_list[j].first)) continue;
                if (!tmp_order.isCoveredBy(edge_list[i].second, edge_list[j].second)) {
                    if (config::is_print) {
                        std::cout << "counter example " << std::endl; order->print();
                        std::cout << data::dataList2String(env) << std::endl;
                        printEdge(edge_list[i]);
                        for (auto& ne: edge_list[i].second) {
                            auto plan = plan_list[ne];
                            for (auto* cmp: order->cmp_list) {
                                std::cout << cmp->key->run({plan}, env).getInt() << ",";
                            }
                            std::cout << " ";
                        }
                        std::cout << std::endl;
                        printEdge(edge_list[j]);
                        for (auto& ne: edge_list[j].second) {
                            auto plan = plan_list[ne];
                            for (auto* cmp: order->cmp_list) {
                                std::cout << cmp->key->run({plan}, env).getInt() << ",";
                            }
                            std::cout << " ";
                        }
                        std::cout << std::endl;
                        int kk; std::cin >> kk;
                    }
                    res.insert({edge_list[i].first, edge_list[j].first});
                    if (res.size() == max_num) {
                        return setToList(res);
                    }
                }
            }
        }
    }
    return setToList(res);
}

bool PlanExecutionLog::verify(CmpPreOrder *order) {
    return extractLocalExample(order, 1).empty();
}

int PlanExecutionLog::getCoveredNum(CmpProgram *cmp, const std::vector<std::pair<int, int>> &example_list) {
    int res = 0;
    auto type = cmp->type;
    auto* oup = getOutputs(cmp->key);
    for (const auto& example: example_list) {
        if (!program::applyCmp(type, oup->at(example.first).getInt(), oup->at(example.second).getInt())) {
            ++res;
        }
    }
    return res;
}

PlanExecutionLog::~PlanExecutionLog() {
    for (auto& cache_item: evaluate_cache) {
        delete cache_item.second;
    }
}

NegCmpExample PlanExecutionLog::buildExample(const std::pair<int, int> &local_example) {
    return {plan_list[local_example.first], plan_list[local_example.second], env};
}

std::string PlanExecutionLog::getFeature(CmpProgram *cmp) {
    auto p_feature = cmp->toString();
    if (feature_map.count(p_feature)) return feature_map[p_feature];
    std::string feature = cmp->isEq() ? "0" : "1";
    auto* now = getOutputs(cmp->key);
    std::vector<int> id;
    for (int i = 0; i < now->size(); ++i) id.push_back(i);
    std::sort(id.begin(), id.end(), [=](int x, int y) {return now->at(x).getInt() < now->at(y).getInt();});
    if (cmp->type == GEQ) std::reverse(id.begin(), id.end());
    for (int i = 1; i < id.size(); ++i) {
        if (now->at(id[i]).getInt() == now->at(id[i - 1]).getInt()) feature += "|"; else feature += ",";
        feature += std::to_string(id[i]);
    }
    return feature_map[p_feature] = feature;
}