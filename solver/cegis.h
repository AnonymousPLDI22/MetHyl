//
// Created by pro on 2021/9/16.
//

#ifndef DPSYNTHESISNEW_CEGIS_H
#define DPSYNTHESISNEW_CEGIS_H

#include "solver/grammar.h"
#include "enumerator.h"

class CEGISExample {
public:
    virtual bool isValid(Program* program) const = 0;
};

class CEGISVerifier: public Verifier {
public:
    std::vector<CEGISExample*> example_list;
    CEGISVerifier(const std::vector<CEGISExample*>& _example_list);
    bool isValid(Program* program);
};

namespace solver {
    Program* trivialCEGIS(const std::vector<CEGISExample*>& example_list, Grammar* g, Optimizer* optimizer = nullptr);
}


#endif //DPSYNTHESISNEW_CEGIS_H
