#ifndef DCA_PLUGIN_HPP
#define DCA_PLUGIN_HPP

// clang-format off
#ifdef _MSC_VER
    #define MY_EXPORT __declspec(dllexport)
#else
    #define MY_EXPORT
#endif

extern "C" {
MY_EXPORT void computeSparseEigenVector(const double* sigmaData,int n, int k, double* sparseEigenVectorData);
}

// clang-format on
#endif // DCA_PLUGIN_HPP
