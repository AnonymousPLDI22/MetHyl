//
// Created by pro on 2021/9/14.
//

#include "input_sampler.h"
#include "config.h"
#include "glog/logging.h"
#include <iostream>
#include <cassert>

namespace {
    int getParamNum(Type* type) {
        int res = 0;
        if (dynamic_cast<LimitedType*>(type)) res += 1;
        for (auto* sub_type: type->param) {
            res += getParamNum(sub_type);
        }
        return res;
    }

    Type* squeeze(Type* type, const std::vector<double>& alpha_list, int& pos) {
        auto* lt = dynamic_cast<LimitedType*>(type);
        double alpha = 0;
        if (lt) {
            alpha = alpha_list[pos];
            pos++;
        }
        std::vector<Type*> param_list;
        for (auto* sub: type->param) {
            param_list.push_back(squeeze(sub, alpha_list, pos));
        }
        if (lt) return lt->squeeze(alpha, param_list);
        return new Type(type->type, param_list);
    }

    Type* squeeze(Type* type, const std::vector<double>& alpha_list) {
        int pos = 0;
        return squeeze(type, alpha_list, pos);
    }
}

TaskInp InputSampler::sampleTaskExample() const {
    Data example_state = s->sampleData(sample_state_type, "");
    DataList example_env;
    for (int i = 0; i < sample_env_type.size(); ++i) {
        example_env.push_back(s->sampleData(sample_env_type[i], t->env_list[i].name));
    }
    return {example_state, example_env};
}

void InputSampler::setSampleType(const AlphaList &state_alpha, const AlphaStorage &env_alpha) {
    sample_state_type = squeeze(t->state_type, state_alpha);
    sample_env_type.clear();
    for (int i = 0; i < env_alpha.size(); ++i) {
        if (t->env_list[i].name.find("fs_") != std::string::npos)
            sample_env_type.push_back(dynamic_cast<LimitedType*>(t->env_list[i].type));
        else
            sample_env_type.push_back(squeeze(t->env_list[i].type, env_alpha[i]));
    }
    // std::cout << sample_state_type->getName() << " " << type::typeList2String(sample_env_type) << std::endl;
}

bool InputSampler::isValidAlpha(const AlphaList &state_alpha, const AlphaStorage &env_alpha) {
    setSampleType(state_alpha, env_alpha);
    bool is_all_empty = false;
    for (int _ = 0; _ < config::KSamplerTestNum; ++_) {
        auto example_inp = sampleTaskExample();
        auto res = e->execute(example_inp.first, example_inp.second, config::KSamplerTimeOut);
        if (res < -config::KINF) return false;
        if (res > config::KINF) is_all_empty = true;
    }
    return !is_all_empty;
}

InputSampler::InputSampler(Task *_t, Sampler *_s, Type *_sample_state_type, const TypeList &_sample_env_type):
    e(new Executor(_t)), t(_t), s(_s), sample_state_type(_sample_state_type), sample_env_type(_sample_env_type) {
}

InputSampler::InputSampler(Task *_t, Executor* _e, Sampler *_s): e(_e), t(_t), s(_s) {
    if (s == nullptr) s = new UniformSampler();
    if (e == nullptr) e = new Executor(_t);
    std::vector<double> inp_alpha_list(getParamNum(t->state_type));
    std::vector<std::vector<double>> env_alpha_storage;
    for (auto& env_info: t->env_list) {
        env_alpha_storage.emplace_back(getParamNum(env_info.type));
    }
    double alpha = 1.0;
    while (1) {
        for (auto& w: inp_alpha_list) w = alpha;
        for (auto& l: env_alpha_storage) {
            for (auto& w: l) w = alpha;
        }
        if (isValidAlpha(inp_alpha_list, env_alpha_storage)) break;
        alpha *= 0.9;
        if (alpha < 1e-5) {
            std::cout << "Fail to generate sample type with alpha " << alpha << std::endl;
            assert(0);
        }
    }
    while (1) {
        bool is_updated = false;
        for (auto& w: inp_alpha_list) {
            if (w > 0.9) continue;
            double pre = w; w = 1.0;
            if (!isValidAlpha(inp_alpha_list, env_alpha_storage)) w = pre; else is_updated = true;
        }
        for (auto& l : env_alpha_storage) {
            for (auto& w: l) {
                if (w > 0.9) continue;
                double pre = w; w = 1.0;
                if (!isValidAlpha(inp_alpha_list, env_alpha_storage)) w = pre; else is_updated = true;
            }
        }
        if (!is_updated) break;
    }
    setSampleType(inp_alpha_list, env_alpha_storage);
}

void InputSampler::printType(FILE* file) const {
    std::cout << "State type: " << sample_state_type->getName() << std::endl;
    std::cout << "Env type:";
    for (auto* env_type: sample_env_type) std::cout << " " << env_type->getName();
    std::cout << std::endl;
    if (file) {
        fprintf(file, "State type: %s\n", sample_state_type->getName().c_str());
        fprintf(file, "Env type: %s\n", type::typeList2String(sample_env_type).c_str());
    }
}

Type * SamplerLimitConfig::getLimitedType(Type *t, bool is_out) const {
    TypeList sub_list;
    for (auto* sub_t: t->param) {
        sub_list.push_back(getLimitedType(sub_t, false));
    }
    auto* lt = dynamic_cast<LimitedType*>(t);
    if (lt) {
        auto* li = dynamic_cast<LimitedInt*>(t);
        if (li) {
            if (is_out) {
                return new LimitedInt(std::max(li->l, out_int_min), std::min(li->r, out_int_max));
            } else {
                return new LimitedInt(std::max(li->l, in_int_min), std::min(li->r, in_int_max));

            }
        }
        auto* ls = dynamic_cast<SizeLimitedDS*>(t);
        if (ls) {
            return new SizeLimitedDS(ls->type, std::min(ls->size, size_limit), sub_list);
        }
        assert(0);
    }
    return new Type(t->type, sub_list);
}


InputSampler * InputSampler::getLimitedSampler(const SamplerLimitConfig &c) {
    auto* limited_state_type = c.getLimitedType(sample_state_type, true);
    TypeList limited_env_type;
    for (auto* env_type: sample_env_type) {
        limited_env_type.push_back(c.getLimitedType(env_type, true));
    }
    return new InputSampler(t, s, limited_state_type, limited_env_type);
}

ExecutionLog * InputSampler::getLog(double time_out) const {
    while (1) {
        auto inp = sampleTaskExample();
        auto *execute_log = e->getLog(inp.first, inp.second, time_out);
        if (execute_log) {
            return execute_log;
        } else delete execute_log;
    }
}