#include "userplugin.hpp"

#include <algorithm>
#include <cmath>

void computeSparseEigenVector(const double *sigmaData, int n, int k,
                              double *sparseEigenVectorData)
{
    // assuming sparseEigenVectorData correctly allocated and 0 <= k <= n.
    // dummy solver: 1/sqrt(k) on k first coordinates and 0 otherwise
    const auto value = 1.0 / std::sqrt(static_cast<double>(k));
    std::fill(sparseEigenVectorData, sparseEigenVectorData + k, value);
    if (k < n)
    {
        std::fill(sparseEigenVectorData + k, sparseEigenVectorData + n, 0.0);
    }
}
