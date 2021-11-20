//
// Created by pro on 2020/11/20.
//

#include "config.h"
#include "task.h"
#include "unordered_set"
#include <cassert>

using namespace autolifter;

SepCacheItem::SepCacheItem(Program *_f, TaskCache *_tc, DataStorage &&_result): f(_f), tc(_tc), result(_result) {}

DataList SepCacheItem::getSeparateOup(int k) {
    while (result.size() <= k) {
        auto res = tc->executeSeparate(f, k);
        result.push_back(res);
    }
    return result[k];
}

Data MergedCacheItem::getMergedOup(int k, bool is_rerun) {
    if (is_rerun) return tc->executeMerged(f, k);
    while (result.size() <= k) {
        auto res = tc->executeMerged(f, result.size());
        result.push_back(res);
    }
    return result[k];
}

MergedCacheItem::MergedCacheItem(Program* _f, TaskCache *_tc, DataList &&_result): tc(_tc), result(_result), f(_f) {}

TaskCache::TaskCache(Program *_m, Type *_Func, ExampleSpace *_example_space):
    m(_m), F(_Func), example_space(_example_space) {
    example_space->addUsage();
}

TaskCache::~TaskCache() {
    for (auto* item: merged_item_list) delete item;
    for (auto* item: sep_item_list) delete item;
    example_space->delUsage();
}

Data TaskCache::executeMerged(Program* program, int id) {
    auto* example = example_space->getExample(id);
    auto m_oup = m->run(*example);
    //std::cout << data::dataList2String(*example) << std::endl;
    return program->run(data::unfoldProdData(m_oup));
}

DataList TaskCache::executeSeparate(Program *program, int id) {
    auto* example = example_space->getExample(id);
    DataList res;
    for (int i = 0; i < F->param.size(); ++i) {
        if (F->param[i]->type == T_VAR) {
            auto inp = data::unfoldProdData((*example)[i]);
            res.push_back(program->run(inp));
        }
    }
    return res;
}

CacheItem TaskCache::getCacheItem(Program *program) {
    auto s = program->toString();
    if (name_map.count(s)) {
        int id = name_map[s];
        return {merged_item_list[id], sep_item_list[id]};
    }
    return {nullptr, nullptr};
}

void Task::setTarget(Program *_p) {
    p = _p;
    auto p_items = cache->getCacheItem(p);
    if (p_items.first == nullptr) {
        p_items = cache->insertCache(p, {}, {});
    }
    p_mer = p_items.first;
    p_sep = p_items.second;
    config.poly_config.clear();
    config.poly_config.insertComponent(cache->m);
    config.poly_config.insertComponent(p);
}

std::pair<Data, DataList> Task::getPrecondition(int id) {
    // std::cout << "cur " << p_mer << " " << p_sep << std::endl;
    Data diff = p_mer->getMergedOup(id);
    DataList same = p_sep->getSeparateOup(id);
    auto* example = cache->example_space->getExample(id);
    auto* F = cache->F;
    for (int i = 0; i < F->param.size(); ++i) {
        if (F->param[i]->type != T_VAR) {
            same.push_back((*example)[i]);
        }
    }
    /*if (id <= 10) {
        std::cout << "precondition #" << id << " " << data::dataList2String(*example) << std::endl;
        std::cout << diff.toString() << " " << data::dataList2String(same) << std::endl;
    }*/
    return {diff, same};
}

std::pair<Data, DataList> Task::executeProgram(Program *program, int id) {
    auto cache_item = cache->getCacheItem(program);
    if (cache_item.first) {
        return {cache_item.first->getMergedOup(id), cache_item.second->getSeparateOup(id)};
    }
    return {cache->executeMerged(program, id), cache->executeSeparate(program, id)};
}

DataList Task::executeSeparate(Program* program, int id) {
    auto cache_item = cache->getCacheItem(program);
    if (cache_item.first) {
        return cache_item.second->getSeparateOup(id);
    }
    return cache->executeSeparate(program, id);
}

bool Task::evaluate(Program *program, std::pair<int, int> id_pair) {
    try {
        auto res_1 = executeProgram(program, id_pair.first), res_2 = executeProgram(program, id_pair.second);
        return res_1.second != res_2.second;
    } catch (SemanticsError& e) {
        return false;
    }
}

bool Task::isCached(Program *program) {
    auto feature = program->toString();
    return cache->name_map.count(feature) > 0;
}

void Task::insertCache(Program *program, DataList &&merged_res, DataStorage &&separate_res) {
    cache->insertCache(program, std::move(merged_res), std::move(separate_res));
}

CacheItem TaskCache::insertCache(Program *program, DataList &&merged_res, DataStorage &&sep_res) {
    auto feature = program->toString();
#ifdef DEBUG
    assert(name_map.count(feature) == 0);
#endif
    name_map[feature] = sep_item_list.size();
    auto* mer_item = new MergedCacheItem(program, this, std::move(merged_res));
    merged_item_list.push_back(mer_item);
    auto* sep_item = new SepCacheItem(program, this, std::move(sep_res));
    sep_item_list.push_back(sep_item);
    return {mer_item, sep_item};
}

PointExample Task::buildCExample(const ProgramList &lifting_list, int pos) {
    TypeList x_content = {p->oup_type};
    for (auto* lp: lifting_list) x_content.push_back(lp->oup_type);
    Type* x_type = new Type(T_PROD, x_content);
    Data oup;
    try {
        oup = p_mer->getMergedOup(pos, true);
    } catch (SemanticsError& e) {
        return {{}, {}};
    }
    DataList inp;
    auto* example = cache->example_space->getExample(pos);
    auto* F = cache->F;
    int x_id = 0;
    DataStorage separate_list;
    separate_list.push_back(p_sep->getSeparateOup(pos));
    for (auto* p: lifting_list) {
        separate_list.push_back(cache->getCacheItem(p).second->getSeparateOup(pos));
    }
    for (int i = 0; i < F->param.size(); ++i) {
        if (F->param[i]->type == T_VAR) {
            DataList content;
            for (auto& sep_res: separate_list) {
                content.push_back(sep_res[x_id]);
            }
            inp.emplace_back(new ProdValue(content, x_type));
            x_id += 1;
        } else {
            inp.push_back((*example)[i]);
        }
    }
    //if (inp[0].getType()->type == T_PROD && inp[0].getProdContents()[0] != oup)
    //    std::cout << "build " << data::dataList2String(*example) << " " << example::pointExample2String({inp, oup}) << std::endl;
    return {inp, oup};
}

bool Task::checkFEqual(Program *p1, Program *p2) {
    int num = std::min(std::max(p1->size(), p2->size()), config.KVerifyEqSizeMax) * 100;
    for (int i = 0; i < num; ++i) {
        if (executeProgram(p1, i) != executeProgram(p2, i)) return false;
    }
    return true;
}

Task::Task(Program *_m, Type *_F, ExampleSpace *example_space, Grammar* _g, const AutoLifterConfig& _c):
    cache(new TaskCache(_m, _F, example_space)), g(_g), config(_c) {
}


Task * autolifter::buildTask(Program *_m, Type *_F, ExampleSpace *example_space, Grammar *_g) {
    AutoLifterConfig c;
    return new Task(_m, _F, example_space, _g, c);
}