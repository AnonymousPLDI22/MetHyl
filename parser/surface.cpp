//
// Created by pro on 2021/8/26.
//

#include <cstdarg>
#include <cassert>
#include <iostream>

#include "surface.h"
#include "task.h"

typedef std::pair<bool, class::Type*> TypeUnit;
typedef std::pair<std::string, class::Program*> AuxUnit;
typedef std::vector<std::pair<std::string, class::Type*>> EnvRes;
#define makeReturn(tag, type) (new TypeUnit(tag, type))

ParseInfo info;
void ParseInfo::setType(const std::string &name, class::Type *type) {
    param_type_map[name] = type;
}
void ParseInfo::setInpVars(const std::vector<std::string> &name_list) {
    inp_id_map.clear();
    for (int i = 0; i < name_list.size(); ++i) {
        inp_id_map[name_list[i]] = i;
    }
    inp_num = name_list.size();
}
void ParseInfo::setEnvVars(const std::vector<std::string> &name_list) {
    env_id_map.clear();
    for (int i = 0; i < name_list.size(); ++i) {
        env_id_map[name_list[i]] = i;
    }
}
void ParseInfo::setBTreeInternal(class ::Type *type) {
    btree_internal_type = type;
}

namespace {
    TypeList mergeContent(TypeUnit* l, TypeUnit *r, TypeType tag) {
        TypeList content;
        for (auto* t: {l, r}) {
            if (t->first || t->second->type != tag) content.push_back(t->second);
            else {
                for (auto *sub_type: t->second->param) {
                    content.push_back(sub_type);
                }
            }
        }
        return std::move(content);
    }

    class::Program* buildVarProgram(const std::string& name) {
        //std::cout << name << std::endl;
        assert(info.param_type_map.count(name));
        auto* type = info.param_type_map[name];
        class::Semantics* semantics = nullptr;
        if (info.inp_id_map.count(name)) {
            semantics = new ParamSemantics(info.inp_id_map[name], type);
        } else if (info.env_id_map.count(name)) {
            semantics = new ParamSemantics(info.env_id_map[name] + info.inp_num, type);
        } else {
            semantics = new TmpSemantics(name, type);
        }
        return new SemanticsProgram(semantics, {});
    }
}

void* buildType(const std::string& type, int n_child, ...) {
    // std::cout << type << " " << n_child << std::endl;
    va_list ap;
    va_start(ap, n_child);
    if (type == "Int") {
        return makeReturn(true, TINT);
    } else if (type == "Bool") {
        return makeReturn(true, TBOOL);
    } else if (type == "List") {
        return makeReturn(true, new class::Type(T_LIST, {va_arg(ap, TypeUnit*)->second}));
    } else if (type == "BTree") {
        auto* internal = va_arg(ap, TypeUnit*)->second;
        auto* leaf = va_arg(ap, TypeUnit*)->second;
        return makeReturn(true, new class::Type(T_BTREE, {internal, leaf}));
    } else if (type == "Bracket") {
        return makeReturn(true, va_arg(ap, TypeUnit*)->second);
    } else if (type == "Prod") {
        auto* l = va_arg(ap, TypeUnit*);
        auto* r = va_arg(ap, TypeUnit*);
        auto* res = new class::Type(T_PROD, mergeContent(l, r, T_PROD));
        delete l; delete r;
        return makeReturn(false, res);
    } else if (type == "Sum") {
        auto* l = va_arg(ap, TypeUnit*);
        auto* r = va_arg(ap, TypeUnit*);
        auto* res = new class::Type(T_SUM, mergeContent(l, r, T_SUM));
        delete l; delete r;
        return makeReturn(false, res);
    } else if (type == "LInt") {
        int l = va_arg(ap, int);
        int r = va_arg(ap, int);
        return makeReturn(true, new class::LimitedInt(l, r));
    } else if (type == "LList") {
        int size = va_arg(ap, int);
        auto* sub = va_arg(ap, TypeUnit*);
        auto* res = makeReturn(true, new SizeLimitedDS(T_LIST, size, {sub->second}));
        delete sub;
        return res;
    } else if (type == "LBTree") {
        int size = va_arg(ap, int);
        auto* internal = va_arg(ap, TypeUnit*);
        auto* leaf = va_arg(ap, TypeUnit*);
        auto* res = makeReturn(true, new SizeLimitedDS(T_BTREE, size, {internal->second, leaf->second}));
        delete internal; delete leaf;
        return res;
    } else if (type == "Extract") {
        return va_arg(ap, TypeUnit*)->second;
    } else if (type == "Void") {
        return makeReturn(true, TVOID);
    } else if (type == "X") {
        return makeReturn(true, TVARA);
    }
    assert(0);
}

void* buildProgram(const std::string& type, int n_child, ...) {
    // std::cout << "prog " << type << " " << n_child << std::endl;
    va_list ap;
    va_start(ap, n_child);
    if (type == "Skip") {
        return new EmptyProgram();
    } else if (type == "Semicolon") {
        if (n_child == 1) return new SemicolonProgram({va_arg(ap, class::Program*)});
        auto* x = va_arg(ap, class::SemicolonProgram*);
        auto* y = va_arg(ap, class::Program*);
        x->stmt_list.push_back(y);
        return x;
    } else if (type == "Var") {
        auto name = std::string(va_arg(ap, char*));
        return buildVarProgram(name);
    } else if (type == "If") {
        auto* cond = va_arg(ap, class::Program*);
        auto* tb = va_arg(ap, class::Program*);
        if (n_child == 2) {
            return new IfProgram(cond, tb);
        } else {
            auto* fb = va_arg(ap, class::Program*);
            return new IfProgram(cond, tb, fb);
        }
    } else if (type == "ParamList") {
        if (n_child == 1) {
            auto* param = va_arg(ap, class::Program*);
            auto* res = new ProgramList();
            res->push_back(param);
            return res;
        } else {
            auto* res = va_arg(ap, ProgramList*);
            auto* param = va_arg(ap, class::Program*);
            res->push_back(param);
            return res;
        }
    } else if (type == "Call") {
        std::string name = std::string(va_arg(ap, char*));
        auto* param_list = va_arg(ap, ProgramList*);
        class::Program* res = nullptr;
        if (info.param_type_map.count(name) > 0) {
            auto* pf = buildVarProgram(name);
            res = new ApplyProgram(pf, *param_list);
        } else {
            auto* sem = semantics::string2Semantics(name);
            res = new SemanticsProgram(sem, *param_list);
        }
        delete param_list;
        return res;
    } else if (type == "Callsep") {
        std::string name = std::string(va_arg(ap, char*));
        ProgramList param_list;
        for (int i = 1; i < n_child; ++i) {
            param_list.push_back(va_arg(ap, class::Program*));
        }
        return new SemanticsProgram(semantics::string2Semantics(name), param_list);
    } else if (type == "ForEach") {
        auto* aux = va_arg(ap, AuxUnit*);
        std::string name = aux->first;
        auto* range = aux->second;
        auto* stmt = va_arg(ap, class::Program*);
        auto* res = new ForEachProgram(name, range, stmt);
        delete aux;
        return res;
    } else if (type == "ForEachAux") {
        std::string name = std::string(va_arg(ap, char*));
        auto* range = va_arg(ap, class::Program*);
        assert(range->oup_type->type == T_LIST);
        info.setType(name, range->oup_type->param[0]);
        return new AuxUnit(name, range);
    } else if (type == "Let") {
        auto* l_res = va_arg(ap, AuxUnit*);
        auto* content = va_arg(ap, class::Program*);
        auto name = l_res->first; auto* def = l_res->second;
        return new LetProgram(name, def, content);
    } else if (type == "LetAux") {
        std::string name = std::string(va_arg(ap, char*));
        auto* def = va_arg(ap, class::Program*);
        info.setType(name, def->oup_type);
        return new AuxUnit(name, def);
    } else if (type == "ArgList") {
        ParamList* res = nullptr;
        if (n_child == 3) {
            res = va_arg(ap, ParamList*);
        } else {
            res = new ParamList();
        }
        std::string name = std::string(va_arg(ap, char*));
        auto* var_type = va_arg(ap, class::Type*);
        info.setType(name, var_type);
        res->emplace_back(name, var_type);
        return res;
    } else if (type == "SetInp") {
        auto* param_list = va_arg(ap, ParamList*);
        std::vector<std::string> name_list;
        for (auto& param: *param_list) {
            name_list.push_back(param.first);
        }
        info.setInpVars(name_list);
        return param_list;
    } else if (type == "Lambda") {
        auto* param_list = va_arg(ap, ParamList*);
        auto* content = va_arg(ap, class::Program*);
        auto* res = new LambdaProgram(*param_list, content);
        delete param_list;
        return res;
    } else if (type == "Prod") {
        auto* param_list = va_arg(ap, ProgramList*);
        auto* res = new ProdProgram(*param_list);
        delete param_list;
        return res;
    } else if (type == "Int") {
        int val = va_arg(ap, int);
        return new SemanticsProgram(new ConstSemantics(val), {});
    } else if (type == "Empty") {
        return new SemanticsProgram(new ConstSemantics(Data()), {});
    } else if (type == "Access") {
        auto name = std::string(va_arg(ap, char*));
        auto* s = static_cast<class::Program*>(buildProgram("Var", 1, name.c_str()));
        int ind = va_arg(ap, int);
        return new AccessProgram(s, ind);
    }
    assert(0);
}

void* buildTask(const std::string& type, int n_child, ...) {
    // std::cout << type << " " << n_child << std::endl;
    va_list ap;
    va_start(ap, n_child);
    if (type == "Env") {
        if (n_child == 0) return new EnvRes();
        auto* res = va_arg(ap, EnvRes*);
        std::string name = std::string(va_arg(ap, char*));
        auto* var_type = va_arg(ap, class::Type*);
        res->emplace_back(name, var_type);
        info.setType(name, var_type);
        return res;
    } else if (type == "SetEnv") {
        auto* env_list = va_arg(ap, EnvRes*);
        std::vector<std::string> name_list;
        for (auto& env: *env_list) {
            name_list.push_back(env.first);
        }
        info.setEnvVars(name_list);
        return env_list;
    } else if (type == "Prog") {
        ProgramList* res = nullptr;
        if (n_child == 2) {
            res = va_arg(ap, ProgramList*);
        } else {
            res = new ProgramList();
        }
        auto* prog = va_arg(ap, class::Program*);
        res->push_back(prog);
        return res;
    } else if (type == "Task") {
        auto* state_type = va_arg(ap, class::Type*);
        auto* env_list = va_arg(ap, EnvRes*);
        auto* plan_type = va_arg(ap, class::Type*);
        auto* trans_type = va_arg(ap, class::Type*);
        auto* t = va_arg(ap, class::Program*);
        auto* f_list = va_arg(ap, ProgramList*);
        auto* eval = va_arg(ap, class::Program*);
        auto* sample_example_list = va_arg(ap, TaskExampleList*);
        auto* task = new Task(state_type, *env_list, plan_type, trans_type, t, *f_list, eval, *sample_example_list);
        delete env_list; delete f_list; delete sample_example_list;
        return task;
    }
    assert(0);
}

void* buildExample(const std::string& type, int n_child, ...) {
    va_list ap;
    va_start(ap, n_child);
    if (type == "Int") {
        return new Data(va_arg(ap, int));
    } else if (type == "Empty") {
        return new Data();
    } else if (type == "Prod") {
        auto* value_list = va_arg(ap, DataList*);
        auto* res = new Data(new ProdValue(*value_list));
        delete value_list;
        return res;
    } else if (type == "List") {
        auto* value_list = va_arg(ap, DataList*);
        auto* res = new Data(new ListValue(*value_list));
        delete value_list;
        return res;
    } else if (type == "ValList") {
        if (n_child == 0) return new DataList();
        auto* res = va_arg(ap, DataList*);
        auto* data = va_arg(ap, Data*);
        res->push_back(*data);
        delete data;
        return res;
    } else if (type == "ExampleList") {
        if (n_child == 0) return new TaskExampleList();
        auto* res = va_arg(ap, TaskExampleList*);
        auto* example = va_arg(ap, TaskExample*);
        res->push_back(*example);
        delete example;
        return res;
    } else if (type == "Example") {
        auto* inp = va_arg(ap, Data*);
        auto* env = va_arg(ap, DataList*);
        auto* oup = va_arg(ap, Data*);
        auto* example = new TaskExample(*inp, *env, *oup);
        delete inp; delete env; delete oup;
        return example;
    } else if (type == "EmptyList") {
        auto* t = va_arg(ap, class::Type*);
        return new Data(new ListValue({}, new class::Type(T_LIST, {t})));
    } else if (type == "BTree") {
        if (n_child == 3) {
            auto* v = va_arg(ap, Data*);
            auto* l = va_arg(ap, Data*);
            auto* r = va_arg(ap, Data*);
            auto* res = new Data(new BTreeValue(*v, *l, *r));
            delete v; delete l; delete r;
            return res;
        } else if (n_child == 2) {
            auto* it = va_arg(ap, class::Type*);
            auto* v = va_arg(ap, Data*);
            auto* res = new Data(new BTreeValue(*v, Data(), Data(), new class::Type(T_BTREE, {it, v->getType()})));
            delete v;
            return res;
        }
        assert(0);
    }
    assert(0);
}