//
// Created by pro on 2021/8/27.
//

#include "program.h"
#include "config.h"
#include "complex_value.h"
#include <cassert>
#include <iostream>
#include <unordered_set>

int Program::size() const {
    int size = 1;
    for (auto* sub: getSubPrograms()) {
        size += sub->size();
    }
    return size;
}

Data Program::run(const DataList &inp) const {
    ExecuteInfo info(&inp);
    // std::cout << toString() << " " << data::dataList2String(inp) << std::endl;
    return run(info);
}

Data Program::run(const DataList &inp, const DataList &env) const {
    return run(data::mergeDataList(inp, env));
}

void Program::addRef() {ref++;}
void Program::delRef() {
    ref--;
    if (ref == 0) delete this;
}

class ::Program * Program::copy() {
    addRef(); return this;
}

CollectRes Program::collect(const DataList &inp) const {
    ExecuteInfo info(&inp);
    run(info);
    return std::move(info.collect_res);
}

CollectRes Program::collect(const DataList &inp, const DataList &env) const {
    return collect(data::mergeDataList(inp, env));
}

Data SemanticsProgram::run(ExecuteInfo &info) const {
    DataList sub_result;
    for (auto* sub_program: sub_list) {
        sub_result.push_back(sub_program->run(info));
    }
    //if (config::is_print)
    //std::cout << "run " << this->toString() << " " << semantics->run(std::move(sub_result), info).toString() << std::endl;
    return semantics->run(std::move(sub_result), info);
}

std::string SemanticsProgram::toString() const {
    std::string name = semantics->name;
    for (int i = 0; i < sub_list.size(); ++i) {
        name += (i ? ',' : '(') + sub_list[i]->toString();
    }
    if (!sub_list.empty()) name += ")";
    return name;
}

SemanticsProgram::SemanticsProgram(Semantics *_semantics, const std::vector<Program *> &_sub_list):
    Program(nullptr), sub_list(_sub_list) {
    std::vector<Type*> inp_type_list;
    for (auto* p: sub_list) inp_type_list.push_back(p->oup_type);
    for (auto* p: sub_list) p->addRef();
    semantics = _semantics->concretion(inp_type_list);
    oup_type = semantics->oup_type;
}
SemanticsProgram::~SemanticsProgram() {
    for (auto* p: sub_list) p->delRef();
}

class::Program* SemanticsProgram::clone(const ProgramList &_sub_list) {
    if (semantics->is_polymorphic) {
        for (int i = 0; i < sub_list.size(); ++i) {
            if (!type::equal(sub_list[i]->oup_type, _sub_list[i]->oup_type)) {
                return new SemanticsProgram(semantics::string2Semantics(semantics->name), _sub_list);
            }
        }
    }
    return new SemanticsProgram(semantics, _sub_list);
}

std::string LambdaProgram::toString() const {
    std::string res = "(\\";
    for (int i = 0; i < param_list.size(); ++i) {
        res += " " + param_list[i].first + "@" + param_list[i].second->getName();
    }
    res += " -> " + content->toString() + ")";
    return res;
}

LambdaProgram::LambdaProgram(const ParamList &_param_list, Program *_content): Program(nullptr), content(_content),
    param_list(_param_list) {
    TypeList param_type;
    for (const auto& param: param_list) param_type.push_back(param.second);
    param_type.push_back(content->oup_type);
    oup_type = new Type(T_ARROW, param_type);
    content->addRef();
}
LambdaProgram::~LambdaProgram() {
    content->delRef();
}

class::Program* LambdaProgram::clone(const ProgramList &sub_list) {
    assert(sub_list.size() == 1);
    return new LambdaProgram(param_list, sub_list[0]);
}

Data LambdaProgram::run(ExecuteInfo &info) const {
    auto f = [this](DataList &&inp_list, ExecuteInfo& info) {
#ifdef DEBUG
        assert(inp_list.size() == param_list.size());
#endif
        for (int i = 0; i < inp_list.size(); ++i) {
            auto& param_info = param_list[i];
#ifdef DEBUG
            assert(info[param_info.first].isNull() && type::equal(param_info.second, inp_list[i].getType()));
#endif
            info[param_info.first] = inp_list[i];
        }
        auto res = this->content->run(info);
        for (auto& param_info: param_list) {
            info[param_info.first] = Data();
        }
        return res;
    };
    auto* anonymous_semantics = new AnonymousSemantics("lambda", f, type::extractInpListFromArrow(oup_type), content->oup_type);
    return new ArrowValue(anonymous_semantics, true);
}

LetProgram::LetProgram(const std::string &_var_name, Program *_def, Program *_content):
    Program(_content->oup_type), var_name(_var_name), def(_def), content(_content) {
    def->addRef(); content->addRef();
}
LetProgram::~LetProgram() {
    def->delRef(); content->delRef();
}

class ::Program * LetProgram::clone(const ProgramList &sub_list) {
    assert(sub_list.size() == 2);
    return new LetProgram(var_name, sub_list[0], sub_list[1]);
}

std::string LetProgram::toString() const {
    return "let " + var_name + " = " + def->toString() + " in " + content->toString();
}

Data LetProgram::run(ExecuteInfo &info) const {
    auto val = def->run(info);
    info[var_name] = val;
    auto res = content->run(info);
    info[var_name] = Data();
    return res;
}

ForEachProgram::ForEachProgram(const std::string &_var_name, Program* _range, Program *_content):
    content(_content), range(_range), var_name(_var_name), Program(_content->oup_type) {
#ifdef DEBUG
    assert(range->oup_type->type == T_LIST);
#endif
    range->addRef(); content->addRef();
}
ForEachProgram::~ForEachProgram() {
    range->delRef(); content->delRef();
}

Program* ForEachProgram::clone(const ProgramList &sub_list) {
    assert(sub_list.size() == 2);
    return new ForEachProgram(var_name, sub_list[0], sub_list[1]);
}

std::string ForEachProgram::toString() const {
    return "foreach " + var_name + " in " + range->toString() + ", " + content->toString();
}

Data ForEachProgram::run(ExecuteInfo &info) const {
    auto r = range->run(info);
    for (auto& w: r.getList()->value) {
        info[var_name] = w;
        content->run(info);
    }
    info[var_name] = Data();
    return Data();
}

Data IfProgram::run(ExecuteInfo &info) const {
    // std::cout << "if " << cond->toString() << std::endl;
    bool val = cond->run(info).getBool();
    // if (config::is_print) std::cout << cond->toString() << " " << val << std::endl;
    if (val) {
        return tb->run(info);
    } else if (fb) {
        return fb->run(info);
    } else return Data();
}
std::string IfProgram::toString() const {
    auto res = "if (" + cond->toString() + ") then " + tb->toString();
    if (fb) res += " else " + fb->toString();
    return res;
}
IfProgram::IfProgram(Program* _cond, Program* _tb, Program* _fb): cond(_cond), tb(_tb), fb(_fb), Program(TVOID) {
#ifdef DEBUG
    assert(type::equal(cond->oup_type, TBOOL));
#endif
    if (tb && fb && type::equal(tb->oup_type, fb->oup_type)) oup_type = tb->oup_type;
    cond->addRef(); tb->addRef();
    if (fb) fb->addRef();
}
IfProgram::~IfProgram() {
    cond->delRef(); tb->delRef();
    if (fb) fb->delRef();
}

class ::Program * IfProgram::clone(const ProgramList &sub_list) {
    assert(sub_list.size() >= 2 && sub_list.size() <= 3);
    if (sub_list.size() == 2) {
        return new IfProgram(sub_list[0], sub_list[1]);
    } else {
        return new IfProgram(sub_list[0], sub_list[1], sub_list[2]);
    }
}

SemicolonProgram::SemicolonProgram(const std::vector<Program *> &_stmt_list): Program(nullptr), stmt_list(_stmt_list) {
    if (stmt_list.empty()) oup_type = TVOID; else oup_type = stmt_list[stmt_list.size() - 1]->oup_type;
    for (auto* p: stmt_list) p->addRef();
}
Data SemicolonProgram::run(ExecuteInfo &info) const {
    auto res = Data();
    for (auto* stmt: stmt_list) {
        res = stmt->run(info);
    }
    return res;
}
std::string SemicolonProgram::toString() const {
    std::string res;
    for (auto* stmt: stmt_list) {
        res += stmt->toString() + ";";
    }
    return res;
}
SemicolonProgram::~SemicolonProgram() {
    for (auto* p: stmt_list) p->delRef();
}

class ::Program * SemicolonProgram::clone(const ProgramList &sub_list) {
    return new SemicolonProgram(sub_list);
}

class ::Program * EmptyProgram::clone(const ProgramList &sub_list) {
    assert(sub_list.empty());
    return new EmptyProgram();
}

ApplyProgram::ApplyProgram(Program *_f, const ProgramList &_param_list): f(_f), param(_param_list), Program(nullptr) {
    oup_type = type::extractOupFromArrow(f->oup_type);
#ifdef DEBUG
    auto inp_type_list = type::extractInpListFromArrow(f->oup_type);
    assert(param.size() == inp_type_list.size());
    for (int i = 0; i < param.size(); ++i) {
        assert(type::equal(inp_type_list[i], param[i]->oup_type));
    }
#endif
    f->addRef();
    for (auto* p: param) p->addRef();
}
ApplyProgram::~ApplyProgram() {
    f->delRef();
    for (auto* p: param) f->delRef();
}
std::string ApplyProgram::toString() const {
    auto res = f->toString();
    res += "(";
    for (int i = 0; i < param.size(); ++i) {
        if (i) res += ",";
        res += param[i]->toString();
    }
    return res + ")";
}
Data ApplyProgram::run(ExecuteInfo &info) const {
    auto f_res = f->run(info);
    DataList inp_list;
    for (auto* p: param) inp_list.push_back(p->run(info));
    auto* f_v = f_res.getSemantics();
    auto res = f_v->run(std::move(inp_list), info);
    return res;
}

class ::Program * ApplyProgram::clone(const ProgramList &sub_list) {
    assert(!sub_list.empty());
    ProgramList params(sub_list.size() - 1);
    for (int i = 1; i < sub_list.size(); ++i) params[i - 1] = sub_list[i];
    return new ApplyProgram(sub_list[0], params);
}

ProdProgram::ProdProgram(const ProgramList &_param): param(_param), Program(nullptr) {
    TypeList content_types;
    for (auto* p: param) content_types.push_back(p->oup_type);
    oup_type = new Type(T_PROD, content_types);
    for (auto* p: param) p->addRef();
}
ProdProgram::~ProdProgram() {
    for (auto* p: param) p->delRef();
}

std::string ProdProgram::toString() const {
    std::string res = "'(";
    for (int i = 0; i < param.size(); ++i) {
        if (i) res += ",";
        res += param[i]->toString();
    }
    return res + ")";
}

Data ProdProgram::run(ExecuteInfo &info) const {
    DataList res;
    for (auto* p: param) res.push_back(p->run(info));
    return new ProdValue(res, oup_type);
}

class ::Program * ProdProgram::clone(const ProgramList &sub_list) {
    return new ProdProgram(sub_list);
}

AccessProgram::AccessProgram(Program *_s, int _ind): s(_s), ind(_ind - 1), Program(nullptr) {
#ifdef DEBUG
    assert(s->oup_type->type == T_PROD && s->oup_type->param.size() > ind);
#endif
    oup_type = s->oup_type->param[ind];
    s->addRef();
}
AccessProgram::~AccessProgram() {
    s->delRef();
}
std::string AccessProgram::toString() const {
    return s->toString() + "." + std::to_string(ind + 1);
}
Data AccessProgram::run(ExecuteInfo &info) const {
    auto res = s->run(info);
    return res.accessProd(ind);
}

class ::Program * AccessProgram::clone(const ProgramList &sub_list) {
    assert(sub_list.size() == 1);
    return new AccessProgram(sub_list[0], ind + 1);
}

std::string data::collectRes2String(const CollectRes &res) {
    std::string s = "[";
    for (int i = 0; i < res.size(); ++i) {
        if (i) s += ",";
        s += std::to_string(res[i].first) + "@" + res[i].second.toString();
    }
    return s + "]";
}


std::string cmp::cmp2String(CmpType type) {
    switch (type) {
        case LEQ: return "<=";
        case GEQ: return ">=";
        case EQ: return "=";
    }
}

namespace {
    bool cmpData(CmpType type, const Data& x, const Data& y) {
        switch (type) {
            case LEQ: return x <= y;
            case GEQ: return y <= x;
            case EQ: return x == y;
        }
    }
}

std::string CmpProgram::toString() const {
    return cmp::cmp2String(type) + ":" + key->toString();
}

CmpProgram::CmpProgram(CmpType _type, Program *_key): type(_type), key(_key), Program(TBOOL) {
#ifdef DEBUG
    assert(type::equal(key->oup_type, TINT));
#endif
    key->addRef();
}
CmpProgram::~CmpProgram() {
    key->delRef();
}

Data CmpProgram::run(ExecuteInfo &info) const {
    std::cout << "cmpprogram cannot be directly executed" << std::endl;
    assert(0);
}

Data CmpProgram::run(const DataList &x, const DataList &y, const DataList &env) const {
    auto* x_inp = new DataList(data::mergeDataList(x, env)), *y_inp = new DataList(data::mergeDataList(y, env));
    ExecuteInfo x_info(x_inp), y_info(y_inp);
    auto res = new BoolValue(cmpData(type, key->run(x_info), key->run(y_info)));
    delete x_inp; delete y_inp;
    return res;
}

Data CmpProgram::getValue(const DataList &x, const DataList& env) const {
    auto* inp = new DataList(data::mergeDataList(x, env));
    ExecuteInfo info(inp);
    return key->run(info);
}

class ::Program * CmpProgram::clone(const ProgramList &sub_list) {
    assert(sub_list.size() == 1);
    return new CmpProgram(type, sub_list[0]);
}

Program * program::buildAnonymousProgram(AnonymousSemantics *semantics) {
    ProgramList sub_list;
    for (int i = 0; i < semantics->inp_type_list.size(); ++i) {
        auto* type = semantics->inp_type_list[i];
        auto* ps = new ParamSemantics(i, type);
        auto* pp = new SemanticsProgram(ps, {});
        sub_list.push_back(pp);
    }
    return new SemanticsProgram(semantics, sub_list);
}

namespace {
    Program* replaceTmp(Program* prog, const std::string& name, Program* def) {
        auto* sp = dynamic_cast<SemanticsProgram*>(prog);
        if (sp) {
            auto* ts = dynamic_cast<TmpSemantics*>(sp->semantics);
            if (ts && ts->name == name){
                return def->copy();
            }
        }
        auto* lp = dynamic_cast<LetProgram*>(prog);
        if (lp && lp->var_name == name) {
            return prog->copy();
        }
        ProgramList sub_list;
        for (auto* sub: prog->getSubPrograms()) {
            sub_list.push_back(replaceTmp(sub, name, def));
        }
        return prog->clone(sub_list);
    }
}

class ::Program * program::removeAllLet(Program *program) {
    auto* lp = dynamic_cast<LetProgram*>(program);
    if (lp) {
        auto* def = removeAllLet(lp->def);
        auto* mid = replaceTmp(lp->content, lp->var_name, def);
        return removeAllLet(mid);
    }
    ProgramList sub_list;
    for (auto* sub: program->getSubPrograms()) {
        sub_list.push_back(removeAllLet(sub));
    }
    return program->clone(sub_list);
}

class ::Program * program::rewriteParams(class ::Program *program, const ProgramList& param_list) {
    auto* sp = dynamic_cast<SemanticsProgram*>(program);
    if (sp) {
        auto* ps = dynamic_cast<ParamSemantics*>(sp->semantics);
        if (ps && param_list[ps->id]) return param_list[ps->id]->copy();
    }
    ProgramList new_sub_list;
    for (auto* sub: program->getSubPrograms()) {
        new_sub_list.push_back(rewriteParams(sub, param_list));
    }
    return program->clone(new_sub_list);
}

class ::Program * program::buildParam(int id, Type *type) {
    auto* ps = new ParamSemantics(id, type);
    return new SemanticsProgram(ps, {});
}

bool program::isConstant(Program *p) {
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    if (sp) {
        if (dynamic_cast<ParamSemantics*>(sp->semantics)) return false;
    }
    for (auto* sub: p->getSubPrograms()) {
        if (!isConstant(sub)) return false;
    }
    return true;
}

Type* program::extractCollectType(Program* f) {
    auto* sp = dynamic_cast<SemanticsProgram*>(f);
    if (sp) {
        if (sp->semantics->name == "collect") {
            return sp->sub_list[1]->oup_type;
        }
    }
    Type* res = TVOID;
    for (auto* sub: f->getSubPrograms()) {
        auto* sub_res = extractCollectType(sub);
        if (!sub_res) return nullptr;
        if (sub_res->type == T_VOID) continue;
        if (res->type == T_VOID) {
            res = sub_res;
        } else if (!type::equal(res, sub_res)) {
            return nullptr;
        }
    }
    return res;
}

class ::Program * program::buildTmp(const std::string& name, class ::Type *type) {
    auto* tmp_semantics = new TmpSemantics(name, type);
    return new SemanticsProgram(tmp_semantics, {});
}

class ::Program * program::buildConst(const Data &data) {
    auto* const_semantics = new ConstSemantics(data);
    return new SemanticsProgram(const_semantics, {});
}

program::ComponentInfo::ComponentInfo(class ::Type *_type, class ::Program *_program, const std::vector<int> &_trace):
        type(_type), program(_program), trace(_trace){
}

namespace {
    void _extractAllComponents(Type* t, Program* p, std::vector<int>& trace, std::vector<program::ComponentInfo>& res) {
        if (t->type != T_PROD) {
            res.emplace_back(t, program::removeProdAccess(p), trace);
            return;
        }
        for (int i = 0; i < t->param.size(); ++i) {
            trace.push_back(i);
            _extractAllComponents(t->param[i], new AccessProgram(p->copy(), i + 1), trace, res);
            trace.pop_back();
        }
    }
}

std::vector<program::ComponentInfo> program::extractAllComponents(class::Type* type, class ::Program *p) {
    std::vector<ComponentInfo> res;
    std::vector<int> trace;
    _extractAllComponents(type, p, trace, res);
    return res;
}

namespace {
    void _extractUsedTmps(Program* p, std::unordered_set<std::string>& tmp_set, std::unordered_set<std::string>& closed_set, ProgramList& res) {
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp) {
            auto* ts = dynamic_cast<TmpSemantics*>(sp->semantics);
            if (ts) {
                if (tmp_set.find(ts->name) == tmp_set.end() && closed_set.find(ts->name) == closed_set.end()) {
                    res.push_back(p->copy());
                    tmp_set.insert(ts->name);
                }
            }
        }
        auto* lp = dynamic_cast<LambdaProgram*>(p);
        if (lp) {
            for (auto& param: lp->param_list) {
                closed_set.insert(param.first);
            }
        }
        for (auto* sub: p->getSubPrograms()) {
            _extractUsedTmps(sub, tmp_set, closed_set, res);
        }
        if (lp) {
            for (auto& param: lp->param_list) {
                closed_set.erase(param.first);
            }
        }
    }
}

ProgramList program::extractUsedTmps(class ::Program *p) {
    std::unordered_set<std::string> tmp_set;
    std::unordered_set<std::string> closed_tmps;
    ProgramList res;
    _extractUsedTmps(p, tmp_set, closed_tmps,res);
    return res;
}

std::pair<int, Program *> program::unfoldCollect(class ::Program *p) {
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    if (!sp || sp->semantics->name != "collect") return {0, nullptr};
    auto* id = sp->sub_list[0], *val = sp->sub_list[1];
    auto* sid = dynamic_cast<SemanticsProgram*>(id);
    auto* cs = dynamic_cast<ConstSemantics*>(sid->semantics);
    return {cs->value.getInt(), val};
}

class ::Program * program::rewriteProgramWithMap(class ::Program *program, const std::map<std::string, Program *> &map) {
    auto feature = program->toString();
    auto it = map.find(feature);
    if (it != map.end()) {
        return it->second->copy();
    }
    ProgramList sub_list;
    for (auto* sub: program->getSubPrograms()) {
        sub_list.push_back(rewriteProgramWithMap(sub, map));
    }
    return program->clone(sub_list);
}

class ::Program * program::rewriteCollect(class ::Program *p, class ::Program *x, class ::Program *y) {
    if (p == x) return new SemanticsProgram(semantics::string2Semantics("collect"), {program::buildConst(0), y});
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    if (sp && sp->semantics->name == "collect") return new EmptyProgram();
    ProgramList sub_list;
    for (auto* sub: p->getSubPrograms()) {
        sub_list.push_back(rewriteCollect(sub, x, y));
    }
    return p->clone(sub_list);
}

class ::Program * program::buildCollect(int id, class ::Program *content) {
    auto* x = buildConst(id);
    auto* y = content->copy();
    return new SemanticsProgram(semantics::string2Semantics("collect"), {x, y});
}

class ::Program * program::removeProdAccess(class ::Program *p) {
    auto* ap = dynamic_cast<AccessProgram*>(p);
    if (ap) {
        auto* pp = dynamic_cast<ProdProgram*>(ap->s);
        if (pp) {
            return removeProdAccess(pp->param[ap->ind]);
        }
    }
    ProgramList sub_list;
    for (auto* sub: p->getSubPrograms()) {
        sub_list.push_back(removeProdAccess(sub));
    }
    return p->clone(sub_list);
}

bool program::isParam(class ::Program *p) {
    auto* sp = dynamic_cast<SemanticsProgram*>(p);
    if (sp && dynamic_cast<ParamSemantics*>(sp->semantics)) return true;
    return false;
}

std::string program::programList2String(const ProgramList &program_list) {
    std::string res = "[";
    for (int i = 0; i < program_list.size(); ++i) {
        if (i) res += ",";
        res += program_list[i]->toString();
    }
    return res + "]";
}

bool program::applyCmp(CmpType type, const Data &x, const Data &y) {
    return cmpData(type, x, y);
}

namespace {
    void _getIntConsts(Program* p, std::unordered_set<int>& int_set) {
        auto* sp = dynamic_cast<SemanticsProgram*>(p);
        if (sp) {
            auto* cs = dynamic_cast<ConstSemantics*>(sp->semantics);
            if (cs && type::equal(cs->oup_type, TINT)) int_set.insert(cs->value.getInt());
        }
        for (auto* sub: p->getSubPrograms()) {
            _getIntConsts(sub, int_set);
        }
    }
}

std::vector<int> program::getIntConsts(Program *program) {
    std::unordered_set<int> res = {0};
    _getIntConsts(program, res);
    std::vector<int> int_list;
    for (auto w: res) int_list.push_back(w);
    return int_list;
}

bool program::isRef(Program *p) {
    auto *sp = dynamic_cast<SemanticsProgram *>(p);
    if (sp && sp->semantics->name.find("match") != std::string::npos) return true;
    for (auto *sub: p->getSubPrograms()) if (isRef(sub)) return true;
    return false;
}

bool program::isNil(class ::Program *p) {
    return p->toString().find("nli") != std::string::npos || p->toString().find("bleaf") != std::string::npos;
}