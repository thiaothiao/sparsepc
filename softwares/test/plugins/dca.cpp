#include<iostream>

#include"Dca.hpp"

namespace plugin
{
    Dca::Dca()
        : Base( "Dca" ){}

    void Dca::solve(const double* sigmaData,int n, int k, double* solution) const
    {
        std::cout<<"\n Dca::Solve \n";
    }
}// namespace plugin

DEFAULT_SPCA_FACTORY(plugin::Dca)
