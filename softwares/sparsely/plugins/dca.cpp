#include<iostream>

#include"Dca.hpp"

#include "sparsepc/core.hpp"

namespace
{
    using DCA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::DcaModel<double, sparsepc::EigenSolver<double>>>;
}

namespace plugin
{
    Dca::Dca()
        : Base( "Dca" ){}

    void Dca::solve(const double* sigmaData,int n, int k, double* solution) const
    {
        using Index = sparsepc::Index;
        //const DCA::Param param{{static_cast<Index>(k)}};
        //auto components = DCA{ param }.run(Eigen::Map<const sparsepc::Matrix<double>>(sigmaData, n, n));

        for(Index i=0; i<k; ++i)
        {
            solution[i] = 1.0 / std::sqrt(static_cast<double>(k));//components.at(0).vector[i];
        }
        std::cout<<"\n Dca::Solve \n";
    }
}// namespace plugin

DEFAULT_SPCA_FACTORY(plugin::Dca)
