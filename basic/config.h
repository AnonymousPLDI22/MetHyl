//
// Created by pro on 2021/8/29.
//

#ifndef DPSYNTHESISNEW_CONFIG_H
#define DPSYNTHESISNEW_CONFIG_H

#include <string>
#include <vector>

namespace config {
    extern std::string KSourcePath;
    extern const int KINF;
    extern const int KLimitedTypeRangeLimit;
    extern const int KSamplerTestNum;
    extern const int KRelationSolverTestNum;
    extern const double KSampleVerifierTimeOut;
    extern const double KSamplerTimeOut;
    extern const int KSynthesisExampleNum;
    extern const int KSynthesisRandomSampleNum;
    extern const int KInitCmpNum;
    extern const int KInitExampleNum;
    extern const double KPolyGenTimeOut;
    extern const double KAutoLifterTimeOut;
    extern const int KComponentUpperBound;
    extern const int KMaxExamplePerExecution;
    extern const int KMaxExamplePerSearch;
    extern const int KSeedExampleNum;
    extern const int KC;
    extern const int KInitComponentTimeOut;
    extern const int KDatasetSizeLow;
    extern const int KValidExampleNum;
    extern const int KValidExampleRange;
    extern const int KILPInputMax;
    extern const int KTimeOut;

    extern bool is_print;
}

#endif //DPSYNTHESISNEW_CONFIG_H
