//
// Created by pro on 2021/8/23.
//

#include "semantics.h"
#include "complex_value.h"
#include "config.h"
#include <cassert>
#include <iostream>

namespace {
    bool isConcreteType(Type* type) {
        if (type->type == T_VAR) {
            return false;
        }
        for (auto* sub_type: type->param) {
            if (!isConcreteType(sub_type)) return false;
        }
        return true;
    }
}

Semantics::Semantics(const std::string &_name, const TypeList &_inp_type_list, Type *_oup_type, bool _is_polymorphic):
    name(_name), inp_type_list(_inp_type_list), oup_type(_oup_type), is_polymorphic(_is_polymorphic) {
    if (is_polymorphic) {
        is_polymorphic = false;
        for (auto* type: inp_type_list) {
            if (!isConcreteType(type)) {
                is_polymorphic = true;
                break;
            }
        }
    }
}

Semantics * Semantics::concretion(const TypeList &inp_list) {
    if (!is_polymorphic) {
        // std::cout << name << " " << type::typeList2String(inp_list) << " " << type::typeList2String(inp_type_list) << std::endl;
#ifdef DEBUG
        for (int i = 0; i < inp_list.size(); ++i) {
            assert(type::equal(inp_list[i], inp_type_list[i]));
        }
#endif
        return this;
    }
    Substitution sigma;
    //std::cout << type::typeList2String(inp_list) << " " << type::typeList2String(inp_type_list) << " " << name << std::endl;
    assert(type::checkUnify(inp_list, inp_type_list, sigma));
    auto* new_oup_type = type::applySubstitution(oup_type, sigma);
    Semantics* new_sem = this->copy();
    new_sem->inp_type_list = inp_list; new_sem->oup_type = new_oup_type;
    return new_sem;
}

Semantics * Semantics::concretion(const TypeList &inp_list, Type *oup) {
#ifdef DEBUG
    auto* tmp = concretion(inp_list);
    assert(type::equal(tmp->oup_type, oup));
#endif
    auto* new_sem = this->copy();
    new_sem->inp_type_list = inp_list;
    new_sem->oup_type = oup;
    return new_sem;
}

void Semantics::check(const DataList &inp_list) const {
    TypeList inp_types; for (auto& w: inp_list) inp_types.push_back(w.getType());
    //std::cout << name << " " << type::typeList2String(inp_type_list) << " " << type::typeList2String(inp_types) << std::endl;
    //assert(inp_list.size() == inp_type_list.size());
    for (int i = 0; i < inp_list.size(); ++i) {
        assert(type::equal(inp_type_list[i], inp_list[i].getType()));
    }
}

Data Semantics::run(DataList &&inp, ExecuteInfo &info) const {
#ifdef DEBUG
    check(inp);
#endif
    return _run(std::move(inp), info);
}

Data ParamSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
#ifdef DEBUG
    assert(id < info.paramNum());
#endif
    return info[id];
}

Data TmpSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto res = info[name];
#ifdef DEBUG
    assert(!res.isNull());
#endif
    return res;
}

Semantics * AnonymousSemantics::copy() const {assert(0);}
Semantics * ConstSemantics::copy() const {assert(0);}
Semantics * ParamSemantics::copy() const {assert(0);}
Semantics * TmpSemantics::copy() const {assert(0);}

PlusSemantics::PlusSemantics(): Semantics("+", {TINT, TINT}, TINT) {}
Data PlusSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return inp[0].getInt() + inp[1].getInt();
}

MinusSemantics::MinusSemantics(): Semantics("-", {TINT, TINT}, TINT) {}
Data MinusSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return inp[0].getInt() - inp[1].getInt();
}

MaxSemantics::MaxSemantics(): Semantics("max", {TINT, TINT}, TINT) {}
Data MaxSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return std::max(inp[0].getInt(), inp[1].getInt());
}

MinSemantics::MinSemantics(): Semantics("min", {TINT, TINT}, TINT) {}
Data MinSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return std::min(inp[0].getInt(), inp[1].getInt());
}

LeqSemantics::LeqSemantics(): Semantics("<=", {TINT, TINT}, TBOOL) {}
Data LeqSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BoolValue(inp[0].getInt() <= inp[1].getInt());
}

EqSemantics::EqSemantics(): Semantics("==", {TVARA, TVARA}, TBOOL, true) {}
Data EqSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BoolValue(inp[0] == inp[1]);
}

GqSemantics::GqSemantics(): Semantics(">", {TINT, TINT}, TBOOL) {}
Data GqSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BoolValue(inp[0].getInt() > inp[1].getInt());
}

NegSemantics::NegSemantics(): Semantics("neg", {TINT}, TINT) {}
Data NegSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return -inp[0].getInt();
}

IteSemantics::IteSemantics(): Semantics("ite", {TBOOL, TVARA, TVARA}, TVARA, true) {}
Data IteSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    bool val = inp[0].getBool();
    if (val) return inp[1]; else return inp[2];
}

CollectSemantics::CollectSemantics(): Semantics("collect", {TINT, TVARA}, TVOID, true) {}
Data CollectSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    info.collect(inp[0].getInt(), inp[1]);
    return Data();
}

TimesSemantics::TimesSemantics(): Semantics("*", {TINT, TINT}, TINT) {}
Data TimesSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    long long res = inp[0].getInt() * inp[1].getInt();
    if (std::abs(res) > config::KINF) throw SemanticsError();
    return int(res);
}

PowSemantics::PowSemantics(): Semantics("pow", {TINT, TINT}, TINT) {}
Data PowSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    int x = inp[0].getInt(), y = inp[1].getInt();
    if (y < 0) throw SemanticsError();
    long long res = 1;
    for (int i = 1; i <= y; ++i) {
        res *= x;
        if (std::abs(res) > config::KINF) throw SemanticsError();
    }
    return int(res);
}

namespace {
    Type* getListVarA() {
        static Type* type = nullptr;
        if (type) return type;
        return type = new Type(T_LIST, {TVARA});
    }
    Type* getListVarB() {
        static Type* type = nullptr;
        if (type) return type;
        return type = new Type(T_LIST, {TVARB});
    }
    Type* getBTree() {
        static Type* type = nullptr;
        if (type) return type;
        return type = new Type(T_BTREE, {TVARA, TVARB});
    }
}
#define TLISTA getListVarA()
#define TLISTB getListVarB()
#define TBTREE getBTree()

namespace {
    void semAssert(bool b) {
        if (!b) throw SemanticsError();
    }
}

HeadSemantics::HeadSemantics(): Semantics("head", {TLISTA}, TVARA, true) {}
Data HeadSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* val = inp[0].getList();
    semAssert(!val->value.empty());
    return val->value[0];
}

LastSemantics::LastSemantics(): Semantics("last", {TLISTA}, TVARA, true) {}
Data LastSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* val = inp[0].getList();
    semAssert(!val->value.empty());
    return val->value[val->value.size() - 1];
}

TailSemantics::TailSemantics(): Semantics("tail", {TLISTA}, TLISTA, true) {}
Data TailSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* val = inp[0].getList();
    DataList res;
    for (int i = 1; i < val->value.size(); ++i) {
        res.push_back(val->value[i]);
    }
    return new ListValue(res, val->getType());
}

SizeSemantics::SizeSemantics(): Semantics("size", {TLISTA}, TINT, true) {}
Data SizeSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& x = inp[0];
    return x.getList()->size();
}

SumSemantics::SumSemantics(): Semantics("sum", {TVARA}, TINT, true) {}
Data SumSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& x = inp[0];
    switch (x.getType()->type) {
        case T_LIST: {
            int res = 0;
            for (auto& v: x.getList()->value) {
                res += v.getInt();
                semAssert(std::abs(res) < config::KINF);
            }
            return res;
        }
        default: assert(0);
    }
}

ConsSemantics::ConsSemantics(): Semantics("cons", {TVARA, TLISTA}, TLISTA, true) {}
Data ConsSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* v = inp[1].getList();
    DataList res = {inp[0]};
    for (auto& _v: v->value) res.push_back(_v);
    return new ListValue(res, oup_type);
}

NilSemantics::NilSemantics(): Semantics("nil", {TVARA}, TLISTA, true) {}
Data NilSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new ListValue({}, oup_type);
}

MapSemantics::MapSemantics(): Semantics("map", {new Type(T_ARROW, {TVARA, TVARB}), TLISTA}, TLISTB, true) {}
Data MapSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* list = inp[1].getList();
    auto* f = inp[0].getSemantics();
    DataList res;
    for (const auto& d: list->value) {
        res.push_back(f->run({d}, info));
    }
    return new ListValue(res, oup_type);
}

FoldSemantics::FoldSemantics(): Semantics("fold", {new Type(T_ARROW, {TVARA, TVARB, TVARB}), TVARB, TLISTA}, TVARB, true) {}
Data FoldSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& list = inp[2].getList()->value;
    Data res = inp[1];
    auto* f = inp[0].getSemantics();
    for (int i = int(list.size()) - 1; i >= 0; --i) {
        res = f->run({list[i], res}, info);
    }
    return res;
}

ZipSemantics::ZipSemantics(): Semantics("zip", {TLISTA, TLISTB},
        new Type(T_LIST, {new Type(T_PROD, {TVARA, TVARB})}), true) {}
Data ZipSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& a = inp[0].getList()->value;
    auto& b = inp[1].getList()->value;
    DataList res;
    for (int i = 0; i < a.size() && i < b.size(); ++i) {
        res.emplace_back(new ProdValue({a[i], b[i]}, oup_type->param[0]));
    }
    return new ListValue(res, oup_type);
}

XorSemantics::XorSemantics(): Semantics("xor", {TINT, TINT}, TINT) {}
Data XorSemantics::_run(DataList &&inp, ExecuteInfo& info) const {
    int res = inp[0].getInt() ^ inp[1].getInt();
    semAssert(std::abs(res) < config::KINF);
    return res;
}

DotsSemantics::DotsSemantics(): Semantics("..", {TINT, TINT}, new Type(T_LIST, {TINT})) {}
Data DotsSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto l = inp[0].getInt(), r = inp[1].getInt();
    DataList res;
    for (int i = l; i <= r; ++i) res.emplace_back(i);
    return new ListValue(res, oup_type);
}

SqrSemantics::SqrSemantics(): Semantics("sqr", {TINT}, TINT) {}
Data SqrSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    int w = inp[0].getInt();
    long long res = 1ll * w * w;
    semAssert(std::abs(res) <= config::KINF);
    return int(res);
}

InitSemantics::InitSemantics(): Semantics("init", {TLISTA}, TLISTA, true) {}
Data InitSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& v = inp[0].getList()->value;
    DataList res;
    for (int i = 0; i + 1 < v.size(); ++i) res.push_back(v[i]);
    return new ListValue(res, oup_type);
}

PrintSemantics::PrintSemantics(): Semantics("print", {TVARA}, TVOID, true) {}
Data PrintSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    std::cout << "print " << inp[0].toString() << std::endl;
    return Data();
}

AndSemantics::AndSemantics(): Semantics("&&", {TBOOL, TBOOL}, TBOOL) {}
Data AndSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BoolValue(inp[0].getBool() && inp[1].getBool());
}

OrSemantics::OrSemantics(): Semantics("||", {TBOOL, TBOOL}, TBOOL) {}
Data OrSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BoolValue(inp[0].getBool() || inp[1].getBool());
}

TakeSemantics::TakeSemantics(): Semantics("take", {TLISTA, TINT}, TLISTA, true) {}
Data TakeSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& v = inp[0].getList()->value;
    int s = inp[1].getInt();
    DataList res;
    for (int i = 0; i < v.size() && i < s; ++i) res.push_back(v[i]);
    return new ListValue(res, oup_type);
}

DropSemantics::DropSemantics(): Semantics("drop", {TLISTA, TINT}, TLISTA, true) {}
Data DropSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& v = inp[0].getList()->value;
    int s = inp[1].getInt();
    DataList res;
    for (int i = s; i < v.size(); ++i) res.push_back(v[i]);
    return new ListValue(res, oup_type);
}

AccessSemantics::AccessSemantics(): Semantics("access", {TLISTA, TINT}, TVARA, true) {}
Data AccessSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& v = inp[0].getList() -> value;
    int ind = inp[1].getInt();
    int size = v.size();
    if (ind < 0) ind += v.size();
    semAssert(ind >= 0 && ind < v.size());
    return v[ind];
}

SuffixOfSemantics::SuffixOfSemantics(): Semantics("suffix", {TLISTA, TLISTA}, TBOOL, true) {}
Data SuffixOfSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& s = inp[0].getList()->value, &t = inp[1].getList()->value;
    if (s.size() > t.size()) return new BoolValue(false);
    for (int i = 0; i < s.size(); ++i) {
        if (s[i] != t[i]) return new BoolValue(false);
    }
    return new BoolValue(true);
}

LMatchSemantics::LMatchSemantics(): Semantics("lmatch", {TLISTA, TLISTA}, TINT, true) {}
namespace {
    int _listMatch(DataList& ll, DataList& gl) {
        for (int i = 0; i <= int(gl.size()) - int(ll.size()); ++i) {
            bool is_match = true;
            for (int j = 0; j < ll.size(); ++j)
                if (ll[j] != gl[i + j]) {
                    is_match = false; break;
                }
            if (is_match) return i;
        }
        return -1;
    }
}
Data LMatchSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* ll = inp[0].getList(), *gl = inp[1].getList();
    auto pos = _listMatch(ll->value, gl->value);
    if (pos == -1) throw SemanticsError();
    return pos;
}

LMoveSemantics::LMoveSemantics(): Semantics("lmove", {TLISTA, TINT, TINT, TINT}, TINT, true) {}
Data LMoveSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* gl = inp[0].getList(); int pos = inp[1].getInt();
    int l = inp[2].getInt() + pos, r = inp[3].getInt() + pos;
    if (r > gl->size() || l < 0 || l > r) throw SemanticsError();
    DataList cur;
    for (int i = l ; i < r; ++i) cur.push_back(gl->value[i]);
    int res = _listMatch(cur, gl->value);
    return res;
}

/*FlattenSemantics::FlattenSemantics(): Semantics("flatten", {TBTREE}, TLISTA, true) {}
namespace {
    void _collect(BTreeValue* b_val, DataList& res) {
        if (b_val->isLeaf()) {
            if (!b_val->v.isNull()) res.push_back(b_val->v);
            return;
        }
        _collect(b_val->l.getBTree(), res);
        if (!b_val->v.isNull()) res.push_back(b_val->v);
        _collect(b_val->r.getBTree(), res);
    }
}
Data FlattenSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
#ifdef DEBUG
    auto* internal_type = inp[0].getType()->param[0];
    assert(type::equal(internal_type, TVOID) || type::equal(internal_type, inp[0].getType()->param[1]));
#endif
    DataList res;
    _collect(inp[0].getBTree(), res);
}*/

BNodeSemantics::BNodeSemantics(): Semantics("bnode", {TVARA, TBTREE, TBTREE}, TBTREE, true) {}
Data BNodeSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BTreeValue(inp[0], inp[1], inp[2], oup_type);
}

BLeafSemantics::BLeafSemantics(): Semantics("bleaf", {TVARA, TVARB}, TBTREE, true) {}
Data BLeafSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BTreeValue(inp[1], Data(), Data(), oup_type);
}

BIsLeafSemantics::BIsLeafSemantics(): Semantics("isleaf", {TBTREE}, TINT, true) {}
Data BIsLeafSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[0].getBTree();
    if (tree->isLeaf()) return 1; else return 0;
}

BIContentSemantics::BIContentSemantics(): Semantics("bic", {TBTREE}, TVARA, true) {}
Data BIContentSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[0].getBTree();
    if (tree->isLeaf()) throw SemanticsError();
    return tree->v;
}

BLContentSemantics::BLContentSemantics(): Semantics("blc", {TBTREE}, TVARB, true) {}
Data BLContentSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[0].getBTree();
    if (!tree->isLeaf()) throw SemanticsError();
    return tree->v;
}

BContentSemantics::BContentSemantics(): Semantics("bc", {new Type(T_BTREE, {TVARA, TVARA})}, TVARA, true) {}
Data BContentSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return inp[0].getBTree()->v;
}

BLeftSemantics::BLeftSemantics(): Semantics("bl", {TBTREE}, TBTREE, true) {}
Data BLeftSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[0].getBTree();
    if (tree->isLeaf()) throw SemanticsError();
    return tree->l;
}

BRightSemantics::BRightSemantics(): Semantics("br", {TBTREE}, TBTREE, true) {}
Data BRightSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[0].getBTree();
    if (tree->isLeaf()) throw SemanticsError();
    return tree->r;
}

BSizeSemantics::BSizeSemantics(): Semantics("bsize", {TBTREE}, TINT, true) {}
Data BSizeSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[0].getBTree();
    return tree->size();
}

BFoldSemantics::BFoldSemantics(): Semantics("bfold",
        {TBTREE, new Type(T_ARROW, {TVARA, TVARC, TVARC, TVARC}), new Type(T_ARROW, {TVARB, TVARC})},
        TVARC, true) {
}
namespace {
    Data _bfold(BTreeValue* t, Semantics* f, Semantics* g, ExecuteInfo& info) {
        if (t->isLeaf()) return g->run({t->v}, info);
        auto l_res = _bfold(t->l.getBTree(), f, g, info);
        auto r_res = _bfold(t->r.getBTree(), f, g, info);
        return f->run({t->v, l_res, r_res}, info);
    }
}
Data BFoldSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[0].getBTree();
    auto* f = inp[1].getSemantics();
    auto* g = inp[2].getSemantics();
    return _bfold(tree, f, g, info);
}

BMatchSemantics::BMatchSemantics(): Semantics("bmatch", {TBTREE, TBTREE}, TINT, true) {}
namespace {
    int _bTreeMatch(BTreeValue* t, const std::string& f, int &cnt) {
        if (t->toString() == f) return cnt;
        cnt++;
        if (t->isLeaf()) return -1;
        int res =_bTreeMatch(t->l.getBTree(), f, cnt);
        if (res !=-1) return res;
        return _bTreeMatch(t->r.getBTree(), f, cnt);
    }
    int bTreeMatch(BTreeValue* lt, BTreeValue* gt) {
        int cnt = 0;
        return _bTreeMatch(gt, lt->toString(), cnt);
    }
}
Data BMatchSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    int cnt = 0;
    auto* gt = inp[1].getBTree(); auto* lt = inp[0].getBTree();
    int res = bTreeMatch(lt, gt);
    if (res == -1) throw SemanticsError();
    return res;
}

BMoveLSemantics::BMoveLSemantics(): Semantics("bmovel", {TINT, TBTREE}, TINT, true) {}
namespace {
    BTreeValue* getSubTree(BTreeValue* tree, int& id) {
        if (id == 0) return tree;
        id--;
        if (tree->isLeaf()) return nullptr;
        auto* res = getSubTree(tree->l.getBTree(), id);
        if (res) return res;
        return getSubTree(tree->r.getBTree(), id);
    }
}
Data BMoveLSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[1].getBTree();
    int cnt = inp[0].getInt();
    auto* subtree = getSubTree(tree, cnt);
    if (!subtree || subtree->isLeaf()) throw SemanticsError();
    return bTreeMatch(subtree->l.getBTree(), tree);
}

BIdTreeSemantics::BIdTreeSemantics(): Semantics("subtree", {TINT, TBTREE}, TBTREE, true) {}
Data BIdTreeSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[1].getBTree(); int id = inp[0].getInt();
    auto* subtree = getSubTree(tree, id);
    if (!subtree) throw SemanticsError();
    return subtree;
}

BMoveRSemantics::BMoveRSemantics(): Semantics("bmover", {TINT, TBTREE}, TINT, true) {}
Data BMoveRSemantics::_run(DataList &&inp, ExecuteInfo& info) const {
    auto* tree = inp[1].getBTree(); int cnt = inp[0].getInt();
    auto* subtree = getSubTree(tree, cnt);
    if (!subtree || subtree->isLeaf()) throw SemanticsError();
    return bTreeMatch(subtree->r.getBTree(), tree);
}

BAccessSemantics::BAccessSemantics(): Semantics("baccess", {TINT, new Type(T_BTREE, {TVARA, TVARA})}, TVARA, true) {}
Data BAccessSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto* tree = inp[1].getBTree();
    int cnt = inp[0].getInt();
    auto* subtree = getSubTree(tree, cnt);
    if (!subtree) throw SemanticsError();
    return subtree->v;
}

ReverseSemantics::ReverseSemantics(): Semantics("reverse", {TLISTA}, TLISTA, true) {}
Data ReverseSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    auto& v = inp[0].getList()->value;
    DataList res(v.size());
    for (int i = 0; i < v.size(); ++i) {
        res[i] = v[v.size() - i - 1];
    }
    return new ListValue(res, oup_type);
}

IntSemantics::IntSemantics(): Semantics("int", {TINT}, TINT) {}
Data IntSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return inp[0];
}

LqSemantics::LqSemantics(): Semantics("<", {TINT, TINT}, TBOOL) {}
Data LqSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BoolValue(inp[0].getInt() < inp[1].getInt());
}

NotSemantics::NotSemantics(): Semantics("not", {TBOOL}, TBOOL) {}
Data NotSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    return new BoolValue(!inp[0].getBool());
}

AppendSemantics::AppendSemantics(): Semantics("append", {TLISTA, TVARA}, TLISTA, true) {}
Data AppendSemantics::_run(DataList &&inp, ExecuteInfo &info) const {
    DataList list = inp[0].getList()->value;
    list.push_back(inp[1]);
    return new ListValue(list);
}


namespace {
    std::map<std::string, Semantics*> semantics_name_map = {
            {"+", new PlusSemantics()}, {"-", new MinusSemantics()}, {"max", new MaxSemantics()},
            {"min", new MinSemantics()}, {"<=", new LeqSemantics()}, {"neg", new NegSemantics()},
            {"ite", new IteSemantics()}, {"collect", new CollectSemantics()}, {"*", new TimesSemantics()},
            {"head", new HeadSemantics()}, {"tail", new TailSemantics()}, {"size", new SizeSemantics()},
            {"==", new EqSemantics()}, {"cons", new ConsSemantics()}, {"nil", new NilSemantics()},
            {"map", new MapSemantics()}, {"fold", new FoldSemantics()}, {"sum", new SumSemantics()},
            {"zip", new ZipSemantics()}, {"zip", new ZipSemantics()}, {"xor", new XorSemantics()},
            {"..", new DotsSemantics()}, {"init", new InitSemantics()}, {"sqr", new SqrSemantics()},
            {"print", new PrintSemantics()}, {"&&", new AndSemantics()}, {"||", new OrSemantics()},
            {">", new GqSemantics()}, {"take", new TakeSemantics()}, {"drop", new DropSemantics()},
            {"access", new AccessSemantics()}, {"suffix", new SuffixOfSemantics()}, {"bnode", new BNodeSemantics()},
            {"bleaf", new BLeafSemantics()}, {"bfold", new BFoldSemantics()}, {"rev", new ReverseSemantics()},
            {"last", new LastSemantics()}, {"int", new IntSemantics()}, {"<", new LqSemantics()},
            {"bic", new BIContentSemantics()}, {"blc", new BLContentSemantics()}, {"bl", new BLeftSemantics()},
            {"br", new BRightSemantics()}, /*{"bsize", new BSizeSemantics()},*/ {"bc", new BContentSemantics()},
            {"bmatch", new BMatchSemantics()}, {"bmovel", new BMoveLSemantics()}, {"bmover", new BMoveRSemantics()},
            {"baccess", new BAccessSemantics()}, {"lmove", new LMoveSemantics()}, {"lmatch", new LMatchSemantics()},
            {"not", new NotSemantics()}, {"isleaf", new BIsLeafSemantics()}, {"subtree", new BIdTreeSemantics()},
            {"append", new AppendSemantics()}, {"pow", new PowSemantics()}
    };
}

Semantics * semantics::string2Semantics(const std::string &name) {
    assert(semantics_name_map.count(name));
    return semantics_name_map[name];
}