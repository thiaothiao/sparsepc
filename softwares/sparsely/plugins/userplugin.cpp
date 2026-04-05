#include"userplugin.hpp"

#include <cmath>

void computeSparseEigenVector(const double* sigmaData,int n, int k, double* sparseEigenVectorData)
{
    for(int i=0; i<k; ++i)
    {
        sparseEigenVectorData[i] = 1.0 / std::sqrt(static_cast<double>(k));
    }
}
