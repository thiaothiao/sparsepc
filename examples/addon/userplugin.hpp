#pragma once

// clang-format off
#ifdef _MSC_VER
    #define MY_EXPORT __declspec(dllexport)
#else
    #define MY_EXPORT
#endif

extern "C" {
MY_EXPORT void computeSparseEigenVector(const double* featureMatrixData,int n, int k, double* sparseEigenVectorData);
}

// clang-format on
