#pragma once

#include <Eigen/Dense>

namespace Sparsepc
{
    using Index = Eigen::Index;
    using Vectori = Eigen::Matrix<Index, Eigen::Dynamic, 1>;
    template <std::floating_point ScalarType>
    using Vector = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>;
    template <std::floating_point ScalarType>
    using Matrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic,
                                 Eigen::ColMajor>;
    template <std::floating_point ScalarType>
    using RMMatrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic,
                                   Eigen::RowMajor>;
} // namespace Sparsepc
