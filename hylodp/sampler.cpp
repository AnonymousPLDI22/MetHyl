//
// Created by pro on 2021/9/14.
//

#include "sampler.h"
#include "config.h"
#include <cassert>
#include <iostream>

int UniformSampler::getInt(int l, int r) {
    auto u = std::uniform_int_distribution<int>(l, r);
    return u(e);
}

int UniformSampler::getSize(int size_limit) {
    if (size_limit == 0) return 0;
    auto u = std::uniform_int_distribution<int>(1, size_limit);
    return u(e);
}

Data IntBasedSampler::sampleTree(Type *type, int size) {
    if (size == 0) {
        auto* leaf_type = type->param[1];
        return new BTreeValue(sampleData(leaf_type, ""), Data(), Data(), type);
    } else {
        auto* node_type = type->param[0];
        int l_size = getInt(0, size - 1), r_size = size - 1 - l_size;
        return new BTreeValue(sampleData(node_type, ""), sampleTree(type, l_size), sampleTree(type, r_size), type);
    }
}

Data IntBasedSampler::sampleData(Type *type, const std::string& name) {
    if (type->type == T_LIST) {
        auto *dt = dynamic_cast<SizeLimitedDS *>(type);
        assert(dt);
        if (type->param[0]->type == T_LIST) {
            auto* param = dynamic_cast<SizeLimitedDS*>(type->param[0]);
            assert(param);
            int size = getSize(std::min(dt->size, param->size));
            DataList content;
            for (int i = 0; i < size; ++i) {
                DataList subc;
                for (int j = 0; j < size; ++j) {
                    subc.push_back(sampleData(param->param[0], ""));
                }
                content.emplace_back(new ListValue(subc, param));
            }
            return new ListValue(content, type);
        }
        int size = getSize(dt->size);
        if (name.find("fs_") != std::string::npos) size = dt->size;
        DataList content;
        for (int i = 1; i <= size; ++i) {
            content.push_back(sampleData(type->param[0], ""));
        }
        return new ListValue(content, type);
    } else if (type->type == T_BTREE) {
        auto *dt = dynamic_cast<SizeLimitedDS *>(type);
        assert(dt);
        int size = getSize(dt->size);
        auto res = sampleTree(type, size);
        return res;
    } else if (type->type == T_INT) {
        auto *li = dynamic_cast<LimitedInt *>(type);
        assert(li);
        return getInt(li->l, li->r);
    } else if (type->type == T_BOOL) {
        return new BoolValue(getInt(0, 1));
    } else if (type->type == T_PROD) {
        DataList content;
        for (auto* content_type: type->param) {
            content.push_back(sampleData(content_type, ""));
        }
        return new ProdValue(content, type);
    } else {
        std::cout << "Cannot sample from " << type->getName() << std::endl;
        assert(0);
    }
}