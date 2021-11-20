//
// Created by pro on 2021/1/20.
//

#include "sf_solver.h"
#include "enumerator.h"
#include "glog/logging.h"
#include <cassert>

using namespace autolifter;

namespace {
    bool isBetter(const std::vector<Program*>& x, const std::vector<Program*>& y) {
        int total_size = 0;
        for (auto* p: x) total_size += p->size();
        for (auto* p: y) total_size -= p->size();
        return total_size < 0;
    }
    int vis_f = 0;
}

void SfSolver::getMoreComponents() {
    int num = program_space.size();
    if (num == 0) num = 10000; else num <<= 1;
    EnumConfig c(new AllCollectVerifier(), new DefaultOptimizer(), num);
    auto new_program_space = enumerate::synthesis(task->g, c);
#ifdef DEBUG
    for (int i = 0; i < program_space.size(); ++i) {
        assert(program_space[i]->toString() == new_program_space[i]->toString());
    }
#endif
    program_space = new_program_space;
   /* for (auto* p: program_space) {
        auto feature = p->toString();
        auto pos = feature.find("bold");
        if (pos != std::string::npos) {
            std::cout << feature << std::endl;
        }
    }*/
}

HASH SfSolver::getComponentHash(int k) {
    while (hash_pool.size() <= k) {
        hash_pool.push_back((HASH(rand()) << 32u) + rand());
    }
    return hash_pool[k];
}

void SfSolver::addCounterExample(std::pair<int, int> counter_example) {
    for (int i = 0; i < program_info.size(); ++i) {
        if (is_valid[i]) {
            try {
                program_info[i].append(task->evaluate(program_space[i], counter_example));
            } catch (SemanticsError& e) {
                is_valid[i] = false;
            }
        }
    }
    example_list.push_back(counter_example);
    for (auto& infos: info_list) {
        for (auto* info: infos) delete info;
    }
    info_list.clear();
    next_component_id = 0;
    maximal_list.clear();
    global.clear();
    maximal_set.clear();
    last.clear();
}

void SfSolver::addValidExample(int id) {
    for (int i = 0; i < program_info.size(); ++i) {
        if (is_valid[i]) {
            try {
                task->executeProgram(program_space[i], id);
            } catch (SemanticsError& e){
                is_valid[i] = false;
            }
        }
    }
    valid_examples.push_back(id);
    for (auto& infos: info_list) {
        for (auto* info: infos) delete info;
    }
    info_list.clear();
    next_component_id = 0;
    maximal_list.clear();
    global.clear();
    maximal_set.clear();
    last.clear();
}

void SfSolver::initNewProgram() {
    if (program_info.size() == program_space.size()) {
        getMoreComponents();
    }
    int n = program_info.size();
    auto* program = program_space[n];
    program_info.emplace_back();
    bool valid = true;
    try {
        for (auto example: valid_examples) {
            task->executeProgram(program, example);
        }
        for (auto &example: example_list) {
            bool now = task->evaluate(program, example);
            program_info[n].append(now);
        }
    } catch (SemanticsError& e) {
        valid = false;
    }
    is_valid.push_back(valid);
}

SfSolver::SfSolver(Task *_task, TimeGuard* _guard): task(_task), v(new OccamVerifier(task)), guard(_guard) {
}
SfSolver::~SfSolver() {
    delete v;
}

bool MaximalInfoList::isAdd(EnumerateInfo *x) {
    for (int i = num - 1; i >= 0; --i) {
        if (info_list[i]->info.checkCover(x->info)) {
            return false;
        }
    }
    return true;
}

int MaximalInfoList::add(EnumerateInfo *x) {
    int now = 0;
    bool flag = false;
    for (int i = 0; i < num; ++i) {
        vis_f++;
        auto* pre = info_list[i];
        if (pre->ind_list[pre->ind_list.size() - 1] < x->ind_list[0] && (pre->info|x->info).count() == x->info.size()) {
            flag = true;
        }
        auto overlap_num = (pre->info & x->info).count();
        if (overlap_num == x->info.count()) return flag;
        if (overlap_num != pre->info.count()) {
            info_list[now++] = pre;
        }
    }
    if (now == info_list.size()) {
        info_list.push_back(x);
    } else {
        info_list[now] = x;
    }
    num = now + 1;
    return flag;
}

bool SfSolver::isAddExpression(int k, EnumerateInfo* x) {
    if (!maximal_list[k].isAdd(x)) return false;
    if (k > 0) {
        for (auto ind: x->ind_list) {
            auto now_hash = x->h ^ getComponentHash(ind);
            if (maximal_set.find(now_hash) == maximal_set.end()) return false;
        }
    }
    return true;
}

std::pair<EnumerateInfo*, EnumerateInfo*> SfSolver::recoverResult(EnumerateInfo *info) {
    for (auto& infos: info_list) {
        for (auto* x: infos) {
            if ((x->info | info->info).count() == info->info.size()) {
                return {x, info};
            }
        }
    }
    assert(0);
}

std::pair<EnumerateInfo*, EnumerateInfo*> SfSolver::addExpression(int k, EnumerateInfo *info) {
    if (!isAddExpression(k, info)) {
        delete info;
        return {nullptr, nullptr};
    }
    info_list[k].push_back(info);
    maximal_list[k].add(info);
    int status = global.add(info);
    maximal_set.insert(info->h);
    if (status) return recoverResult(info);
    return {nullptr, nullptr};
}

std::pair<EnumerateInfo*, EnumerateInfo*> SfSolver::getNextComposition(int k) {
    int l_id = 0, r_id = 0;
    while (info_list.size() <= k) {
        info_list.emplace_back();
    }
    while (last.size() <= k) {
        last.emplace_back(-1, 0);
    }
    while (maximal_list.size() <= k) {
        maximal_list.emplace_back();
    }
    while (k) {
        if (info_list[k].empty()) l_id = 0, r_id = 0;
        else {
            l_id = last[k].first + 1;
            r_id = last[k].second;
        }
        while (r_id < info_list[k - 1].size() && info_list[k - 1][r_id]->l_id <= l_id) {
            l_id = 0; ++r_id;
        }
        if (r_id >= info_list[k - 1].size()) --k; else break;
    }
    last[k] = std::make_pair(l_id, r_id);
    if (k == 0) {
        vis_f++;
        if (next_component_id == program_info.size()) {
            initNewProgram();
        }
        if (is_valid[next_component_id]) {
            int n = info_list[0].size();
            auto *new_component = new EnumerateInfo(n, n, getComponentHash(next_component_id),
                                                    {next_component_id}, program_info[next_component_id]);
            if (program_info[next_component_id].count() == example_list.size()) {
                return {new_component, nullptr};
            }
            next_component_id += 1;
            return addExpression(k, new_component);
        } else {
            next_component_id += 1;
            return {nullptr, nullptr};
        }
    } else {
        std::vector<int> ind_list = {info_list[0][l_id]->ind_list[0]};
        for (auto ind: info_list[k - 1][r_id]->ind_list) {
            ind_list.push_back(ind);
        }
        Bitset info = info_list[0][l_id]->info | info_list[k - 1][r_id]->info;
        auto* new_component = new EnumerateInfo(l_id, r_id, info_list[0][l_id]->h ^ info_list[k - 1][r_id]->h,
                ind_list, info);
        return addExpression(k, new_component);
    }
}

std::vector<Program *> SfSolver::getProgramList(EnumerateInfo *info) {
    if (!info) return {};
    std::vector<Program*> program_list;
    for (auto ind: info->ind_list) {
        program_list.push_back(program_space[ind]);
    }
    return program_list;
}

std::vector<Program *> SfSolver::synthesisFromExample() {
    if (example_list.size() == 0) return {};
    int extra_turn_num = -1;
    std::vector<Program*> best_result;
    for (int turn_id = 1;; ++turn_id) {
        if (guard && guard->isTimeOut()) throw TimeOutError();
        int k = (turn_id - 1) % task->config.KTermSolverT;
        if (!best_result.empty()) {
            extra_turn_num--;
            k = 0;
        }
        auto info = getNextComposition(k);
        if (info.first) {
            std::vector<Program *> result;
            for (auto p: getProgramList(info.first)) {
                result.push_back(p);
            }
            if (info.second) {
                for (auto p: getProgramList(info.second)) {
                    result.push_back(p);
                }
            }
            if (best_result.empty() || isBetter(result, best_result)) {
                best_result = result;
                extra_turn_num = task->config.KExtraTermRoundNum;
            }
        }
        if (extra_turn_num == 0 || best_result.size() == 1) return best_result;
    }
}

std::pair<int, int> OccamVerifier::verify(const std::vector<Program *> &program_list) {

    data_map.clear();
    std::vector<Program*> complete_program_list = program_list;
    complete_program_list.push_back(task->p);

    int total = 0, pos = start_verify_pos;
    std::vector<bool> is_cached;
    for (auto* program: complete_program_list) is_cached.push_back(task->isCached(program));
    std::vector<std::vector<DataList>> oup_list(complete_program_list.size());

    auto verify = [&](int pos)->std::pair<int, int>{
        std::pair<Data, DataList> example_info;
        try {
            example_info = task->getPrecondition(pos);
        } catch (SemanticsError& e) {
            return {-1, -1};
        }
        try {
            std::string current_feature = data::dataList2String(example_info.second) + "&";
            for (int i = 0; i < complete_program_list.size(); ++i) {
                auto *program = complete_program_list[i];
                auto res = task->executeSeparate(program, pos);
                current_feature += data::dataList2String(res);
                if (!is_cached[i]) oup_list[i].push_back(std::move(res));
            }
            // std::cout << pos << " " << current_feature << " " << example_info.first.toString() << " " << data::dataList2String(*(task->cache->example_space->getExample(pos))) << std::endl;
            auto iterator = data_map.find(current_feature);
            if (iterator == data_map.end()) {
                data_map[current_feature] = std::make_pair(pos, example_info.first);
            } else {
                if (example_info.first != iterator->second.second) {
                    start_verify_pos = pos;
                    return std::make_pair(pos, iterator->second.first);
                }
            }
            return std::make_pair(-1, -1);
        } catch (SemanticsError& e) {
            return {pos, -1};
        }
    };

    while (total < example_num) {
        total += 1;
        auto verify_result = verify(pos);
        if (verify_result.first != -1) return verify_result;
        pos = (pos + 1) % int(example_num);
    }
    int pre_size = example_num;
    std::vector<DataList> tmp(pre_size);
    int verify_limit = std::min(5, task->p->size());
    for (int i = 0; i < complete_program_list.size(); ++i) {
        if (is_cached[i]) continue;
        verify_limit += complete_program_list[i]->size();
        tmp = oup_list[i];
        for ( int j = 0; j < pre_size; ++j) {
            oup_list[i][pos] = tmp[j];
            pos = (pos + 1) % pre_size;
        }
    }
    example_num = std::max(example_num, verify_limit * config::KInitExampleNum);
    if (program::isNil(task->cache->m)) example_num /= 100;
    for (int i = pre_size; i < example_num; ++i) {
        auto verify_result = verify(i);
        if (verify_result.first != -1) return verify_result;
    }

    for (int i = 0; i < complete_program_list.size(); ++i) {
        if (!is_cached[i]) {
            task->insertCache(complete_program_list[i], {}, std::move(oup_list[i]));
        }
    }
    return {-1, -1};
}

bool SfSolver::synthesisFromExisting(const ProgramList &existing_list, ProgramList &tmp) {
    while (1) {
        auto example = v->verify(tmp);
        if (example.first == -1) return true;
        bool is_solved = false;
        if (example.second == -1) {
            std::cout << "error semantics " << data::dataList2String(*(task->cache->example_space->getExample(example.first))) << std::endl;
            return false;
        }
        for (auto* p: existing_list) {
            if (task->evaluate(p, example)) {
                tmp.push_back(p); is_solved = true; break;
            }
        }
        if (!is_solved) return false;
    }
}

std::vector<Program*> SfSolver::synthesis(const ProgramList& existing_list) {
    ProgramList tmp;
    if (synthesisFromExisting(existing_list, tmp)) {
        return tmp;
    }


    int turn_num = 0;
    while (true) {
        turn_num += 1;
        vis_f = 0;
        auto candidate_result = synthesisFromExample();
        /*std::cout<< "Result #" << turn_num << std::endl;
        for (auto* f: candidate_result) {
            std::cout << "  " << f->toString() << std::endl;
        }*/
        auto example = v->verify(candidate_result);
        /*if (example.first != -1) {
            auto* x = task->cache->example_space->getExample(example.first);
            auto* y = task->cache->example_space->getExample(example.second);
            std::cout << "fail " << data::dataList2String(*x) << " " << data::dataList2String(*y) << std::endl;
        }*/
        if (example.first != -1) {
            if (example.second != -1) addCounterExample(example);
            else addValidExample(example.first);
        } else {
            //std::cout << "res " << std::endl;
            //for (auto* p: candidate_result) std::cout << "  " << p->toString() << std::endl;
            return candidate_result;
        }
        //int kk; std::cin >> kk;
    }
}