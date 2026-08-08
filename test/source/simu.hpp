#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <random>

#include <sparsepc/utils/matrix.hpp>

namespace Sparsepc
{
    namespace linearmodel
    {
        template <std::floating_point ScalarType>
        struct SimulationData
        {
            Matrix<ScalarType> v1;
            Matrix<ScalarType> v2;
            Matrix<ScalarType> v3;
            Matrix<ScalarType> observed;
            Matrix<ScalarType> featureMatrix;

            Matrix<ScalarType> centered() const
            {
                const auto n = observed.rows();
                if (n <= 1)
                {
                    return Matrix<ScalarType>::Zero(observed.cols(),
                                                   observed.cols());
                }

                const auto mean = observed.colwise().mean();
                return observed.rowwise() - mean;
            }

            Matrix<ScalarType> covariance() const
            {
                const auto centered = centered();
                return (centered.transpose() * centered);// /static_cast<ScalarType>(n - 1);
            }

            Matrix<ScalarType> theoretical_covariance() const
            {
                constexpr ScalarType varV1 = static_cast<ScalarType>(290);
                constexpr ScalarType varV2 = static_cast<ScalarType>(300);
                constexpr ScalarType covV1V3 = static_cast<ScalarType>(-87);
                constexpr ScalarType covV2V3 = static_cast<ScalarType>(277.5);
                constexpr ScalarType varV3 = static_cast<ScalarType>(283.7875);

                Matrix<ScalarType> latentCov(3, 3);
                latentCov.setZero();
                latentCov(0, 0) = varV1;
                latentCov(1, 1) = varV2;
                latentCov(2, 2) = varV3;
                latentCov(0, 2) = covV1V3;
                latentCov(2, 0) = covV1V3;
                latentCov(1, 2) = covV2V3;
                latentCov(2, 1) = covV2V3;

                const auto cov = (featureMatrix * latentCov * featureMatrix.transpose()).eval();
                Matrix<ScalarType> result = cov;
                result.diagonal().array() += static_cast<ScalarType>(1);
                return result;
            }
        };

        template <std::floating_point ScalarType>
        auto generate_simulation(Index nSamples = 200,
                                           std::uint32_t seed = 42)
            -> SimulationData<ScalarType>
        {
            SimulationData<ScalarType> result;
            result.v1.resize(nSamples, 1);
            result.v2.resize(nSamples, 1);
            result.v3.resize(nSamples, 1);
            result.observed.resize(nSamples, 10);
            result.featureMatrix.resize(10, 3);
            result.featureMatrix.setZero();

            std::mt19937 generator(seed);
            std::normal_distribution<ScalarType> normalV1(
                static_cast<ScalarType>(0),
                std::sqrt(static_cast<ScalarType>(290)));
            std::normal_distribution<ScalarType> normalV2(
                static_cast<ScalarType>(0),
                std::sqrt(static_cast<ScalarType>(300)));
            std::normal_distribution<ScalarType> normalNoise(
                static_cast<ScalarType>(0), static_cast<ScalarType>(1));

            for (Index i = 0; i < nSamples; ++i)
            {
                const auto value1 = normalV1(generator);
                const auto value2 = normalV2(generator);
                const auto value3 =
                    -static_cast<ScalarType>(0.3) * value1 +
                    static_cast<ScalarType>(0.925) * value2 +
                    normalNoise(generator);

                result.v1(i, 0) = value1;
                result.v2(i, 0) = value2;
                result.v3(i, 0) = value3;

                for (Index j = 0; j < 10; ++j)
                {
                    const auto latent = j < 4   ? value1
                                        : j < 8 ? value2
                                                 : value3;
                    result.observed(i, j) = latent + normalNoise(generator);
                }
            }

            for (Index i = 0; i < 4; ++i)
            {
                result.featureMatrix(i, 0) = static_cast<ScalarType>(1);
            }
            for (Index i = 4; i < 8; ++i)
            {
                result.featureMatrix(i, 1) = static_cast<ScalarType>(1);
            }
            for (Index i = 8; i < 10; ++i)
            {
                result.featureMatrix(i, 2) = static_cast<ScalarType>(1);
            }

            return result;
        }
    } // namespace linearmodel
} // namespace Sparsepc
