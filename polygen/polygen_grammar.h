
//
// Created by pro on 2021/10/23.
//

#ifndef DPSYNTHESISNEW_POLYGEN_GRAMMAR_H
#define DPSYNTHESISNEW_POLYGEN_GRAMMAR_H

#include "grammar.h"
#include "polygen_config.h"

namespace polygen {
    Grammar* buildDefaultTermGrammar(TypeList& inp_type, const PolyGenConfig& c, bool is_atom);
    Grammar* buildDefaultConditionGrammar(TypeList& inp_type, const PolyGenConfig& c, const ProgramList& extra_list);
}


#endif //DPSYNTHESISNEW_POLYGEN_GRAMMAR_H
