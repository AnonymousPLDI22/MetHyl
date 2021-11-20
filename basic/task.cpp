//
// Created by pro on 2021/8/29.
//

#include "config.h"
#include "task.h"
#include <cassert>
#include <iostream>

namespace {
    bool checkNoSumType(class::Type* type) {
        if (type->type == T_SUM) return false;
        for (auto* sub_type: type->param) {
            if (!checkNoSumType(sub_type)) return false;
        }
        return true;
    }
}

namespace {
    bool verifyInpType(Program* p, const TypeList& inp_types, const TypeList& env_types) {
        // std::cout << p->toString() << std::endl;
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp) {
            auto* ps = dynamic_cast<ParamSemantics*>(sp->semantics);
            if (ps) {
                int ind = ps->id;
                if (ind < inp_types.size()) return type::equal(inp_types[ind], ps->oup_type);
                ind -= inp_types.size();
                if (ind < env_types.size()) return type::equal(env_types[ind], ps->oup_type);
                return false;
            }
        }
        for (auto* sub_p: p->getSubPrograms()) {
            if (!verifyInpType(sub_p, inp_types, env_types)) return false;
        }
        return true;
    }

    bool verifyCollectTypes(Program* p, const TypeList& collect_types) {
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp) {
            auto* sem = sp->semantics;
            if (sem->name == "collect") {
                int ind = std::stoi(sp->sub_list[0]->toString());
                auto* type = sp->sub_list[1]->oup_type;
                // std::cout << ind << " " << type->getName() << " " << type::typeList2String(collect_types) << std::endl;
                return ind <= collect_types.size() && ind > 0 && type::equal(collect_types[ind - 1], type);
            }
        }
        for (auto* sub_p: p->getSubPrograms()) {
            if (!verifyCollectTypes(sub_p, collect_types)) return false;
        }
        return true;
    }
}

Task::Task(class ::Type *_state, const std::vector<std::pair<std::string, class ::Type *> > &vars, class ::Type *_plan,
        class ::Type *_trans, class ::Program *_t, const ProgramList &_f_list, class ::Program *_eval,
        const TaskExampleList &_sample_example_list): state_type(_state), plan_type(_plan), eval(_eval), t(_t),
        f_list(_f_list), sample_example_list(_sample_example_list) {
    TypeList trans_full_type;
    if (_trans->type != T_SUM) trans_full_type = {_trans};
    else trans_full_type = _trans->param;
    for (auto* type: trans_full_type) {
        assert(checkNoSumType(type));
        TypeList inp_types;
        if (type->type == T_PROD) trans_types.push_back(type->param);
        else trans_types.push_back({type});
    }
    for (auto& var: vars) env_list.emplace_back(var.first, var.second);

    // check
    assert(f_list.size() == trans_types.size());
    TypeList env_types;
    for (auto& env: env_list) env_types.push_back(env.type);
    for (int i = 0; i < f_list.size(); ++i) {
        TypeList inp_types;
        for (auto* type: trans_types[i]) {
            if (type->type == T_VAR) inp_types.push_back(plan_type); else inp_types.push_back(type);
        }
        // std::cout << f_list[i]->toString() << " " << type::typeList2String(inp_types) << std::endl;
        assert(verifyInpType(f_list[i], inp_types, env_types));
    }
    TypeList trans_oup_types;
    for (auto& trans_type: trans_types) {
        if (trans_type.size() == 1) {
            if (trans_type[0]->type == T_VAR) trans_oup_types.push_back(state_type);
            else trans_oup_types.push_back(trans_type[0]);
        } else {
            TypeList sub_types;
            for (auto* type: trans_type) {
                if (type->type == T_VAR) sub_types.push_back(state_type);
                else sub_types.push_back(type);
            }
            trans_oup_types.push_back(new Type(T_PROD, sub_types));
        }
    }
    assert(verifyCollectTypes(t, trans_oup_types));
}

void Task::print(FILE* oup) const {
    std::cout << "State: " << state_type->getName() << "\t" << "Plan: " << plan_type->getName() << std::endl;
    std::cout << "Env:";
    for (auto& env: env_list) std::cout << " " << env.name << "@" << env.type->getName(); std::cout << std::endl;
    std::cout << "Trans:";
    for (auto& trans_type: trans_types) std::cout << " " << type::typeList2String(trans_type); std::cout << std::endl;
    std::cout << "T: " << t->toString() << std::endl;
    std::cout << "F:" << std::endl;
    for (auto* f: f_list) {
        std::cout << "   " << f->toString() << std::endl;
    }
    std::cout << "Example:" << std::endl;
    for (auto& example: sample_example_list) {
        std::cout << "   " << example.inp.toString() << " " << data::dataList2String(example.env) << " -> " << example.oup.toString() << std::endl;
    }
    if (oup) {
        fprintf(oup, "State: %s\t Plan: %s\n", state_type->getName().c_str(), plan_type->getName().c_str());
        fprintf(oup, "T: %s\n", t->toString().c_str());
        fprintf(oup, "F:\n");
        for (auto* f: f_list) fprintf(oup, "\t%s\n", f->toString().c_str());
    }
}

int Task::evaluate(const Data &plan, const DataList &env) const {
    return eval->run({plan}, env).getInt();
}

TypeList Task::getEnvType() const {
    TypeList res;
    for (auto& info: env_list) res.push_back(info.type);
    return res;
}