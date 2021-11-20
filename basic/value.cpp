//
// Created by pro on 2021/8/23.
//

#include "value.h"
#include "data.h"
#include "semantics.h"
#include <cassert>

bool IntValue::equal(Value *v) const {
    auto* iv = dynamic_cast<IntValue*>(v);
    if (!iv) return false;
    return iv->value == value;
}

bool BoolValue::equal(Value* v) const {
    auto* bv = dynamic_cast<BoolValue*>(v);
    if (!bv) return false;
    return bv->value == value;
}