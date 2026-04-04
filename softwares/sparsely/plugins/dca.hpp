#ifndef DCA_PLUGIN_HPP
#define DCA_PLUGIN_HPP

#include <concepts>

#include "spca.hpp"

#include "sparsepc/core.hpp"

namespace plugin
{
    class Dca final : public SPCA
    {
    public:
        using Base = SPCA;

        Dca();

        void solve(const double* sigmaData,int n, int k, double* solution) const override;
    };
}// namespace plugin

#endif// DCA_PLUGIN_HPP
