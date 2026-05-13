#pragma once

#include <concepts>

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/generic/solver.hpp>
#include <sparsepc/progress/bar.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace Sparsepc
{
    namespace linearmodel
    {
        /**
         * @brief A Contiguous support solver class.
         * @details It computes solutions using contiguous subsets of indices
         * as support.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class ContiguousSupportSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            /**
             * @brief ContiguousSupportSolver parameter set.
             * @details Store the parameters needed for the computations.
             */
            struct Param final
            {
                Param(Index kInput = static_cast<Index>(-1),
                      const EigenSolver &eigenSolverInput = {},
                      Scalar zeroInput = static_cast<Scalar>(1e-6))
                    : k{kInput}, eigenSolver{eigenSolverInput}, zero{zeroInput}
                {
                }

                Param(const Param &) = default;
                Param &operator=(const Param &) = default;

                Param(Param &&) = default;
                Param &operator=(Param &&) = default;

                const Index k;
                const EigenSolver eigenSolver;
                const Scalar zero;
            };

            /**
             * @brief Constructs a new ContiguousSupportSolver object with specified
             * parameters.
             * @param param The parameters used by the solver object.
             */
            ContiguousSupportSolver(const Param &param = {}) : m_Param{param} {}

            /**
             * @brief Computes the principal component associated with the
             * parameters.
             * @param sigma The covariance matrix.
             * @return The computed principal component.
             */
            auto run(const Matrix<Scalar> &sigma) const;

            /**
             * @brief Computes a set of principal component candidates.
             * @param sigma The covariance matrix.
             * @param param The parameters to be used.
             * @param progressBar The computation progress reporter.
             * @return The computed candidates.
             */
            static auto run(const Matrix<Scalar> &sigma, const Param &param,
                            ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        ContiguousSupportSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &sigma) const
        {
            using Component = Component<Scalar>;

            const auto k = m_Param.k;
            const auto &eigenSolver = m_Param.eigenSolver;

            const auto n = sigma.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(sigma);
            }

            if (static_cast<Index>(1) == n)
            {
                Component cmponent(n);
                cmponent.value = sigma.value();
                cmponent.vector.setOnes();

                return cmponent;
            }

            const Vectori indices = Vectori::LinSpaced(n, 0, n - 1);
            auto lambdaMax = 0.0;
            Index iMax = 0;
            for (Index i = 0; i < n - k + 1; ++i)
            {
                const auto sub = indices.segment(i, k);
                const auto lambda = eigenSolver.maximumValue(sigma(sub, sub));
                if (lambda > lambdaMax)
                {
                    lambdaMax = lambda;
                    iMax = i;
                }
            }

            const auto subMax = indices.segment(iMax, k);
            auto subDimEigenElement =
                eigenSolver.maximumValueElement(sigma(subMax, subMax));

            Component component(n);
            component.vector(subMax) = subDimEigenElement.vector;
            component.value = subDimEigenElement.value;

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        ContiguousSupportSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &sigma, const Param &param,
            ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;

            const auto &eigenSolver = param.eigenSolver;
            const auto k = param.k;
            const auto n = sigma.cols();

            ComponentsContainer components;
            components.reserve(n);

            if (k > static_cast<Index>(0))
            {
                if (progressBar)
                {
                    progressBar->setValue(n / 2);

                    progressBar->processEvents();

                    if (progressBar->wasCanceled())
                    {
                        return components;
                    }
                }

                components.try_emplace(
                    std::min(k, n),
                    ContiguousSupportSolver<Scalar, EigenSolver, ProgressBar>{param}
                        .run(sigma));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

            components.try_emplace(n, eigenSolver.maximumValueElement(sigma));

            if (n == 1)
            {
                return components;
            }

            if (progressBar)
            {
                progressBar->setValue(1);

                progressBar->processEvents();

                if (progressBar->wasCanceled())
                {
                    return components;
                }
            }

            for (Index k = 1; k < n; ++k)
            {
                components.try_emplace(k, Component(n));
            }

            for (Index k = n - 1; k > 0; --k)
            {
                components.at(
                    k) = ContiguousSupportSolver<Scalar, EigenSolver, ProgressBar>{
                    Param{k}}.run(sigma);

                if (progressBar)
                {
                    progressBar->setValue(n - k + 1);

                    progressBar->processEvents();

                    if (progressBar->wasCanceled())
                    {
                        return components;
                    }
                }
            }

            return components;
        }

        template <std::floating_point ScalarType>
        using ContiguousFacetsFinder = SparsePC<
            ContiguousSupportSolver<ScalarType, EigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace Sparsepc
