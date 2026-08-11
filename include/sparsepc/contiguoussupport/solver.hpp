#pragma once

#include <concepts>
#include <cstdlib>

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
             * @param featureMatrix The colwise feature matrix.
             * @param complementaryProjectionMatrix The complementary
             * projection.
             * @return The computed principal component.
             */
            auto run(const Matrix<Scalar> &featureMatrix,
                     const ComplementaryProjection<Scalar>
                         &complementaryProjectionMatrix) const;

            /**
             * @brief Computes a set of principal component candidates.
             * @param featureMatrix The colwise feature matrix.
             * @param complementaryProjectionMatrix The complementary
             * projection.
             * @param param The parameters to be used.
             * @param progressBar The computation progress reporter.
             * @return The computed candidates.
             */
            static auto run(const Matrix<Scalar> &featureMatrix,
                            const ComplementaryProjection<Scalar>
                                &complementaryProjectionMatrix,
                            const Param &param, ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        ContiguousSupportSolver<ScalarType, EigenSolverType, ProgressBarType>::
            run(const Matrix<Scalar> &featureMatrix,
                const ComplementaryProjection<Scalar>
                    &complementaryProjectionMatrix) const
        {
            using Component = Component<Scalar>;
            using Matrix = Matrix<Scalar>;

            const Matrix deflatedFeatureMatrix =
                featureMatrix * complementaryProjectionMatrix;

            const auto k = m_Param.k;
            const auto &eigenSolver = m_Param.eigenSolver;

            const auto n = deflatedFeatureMatrix.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(deflatedFeatureMatrix);
            }

            if (static_cast<Index>(1) == n)
            {
                Component cmponent(n);
                cmponent.value = deflatedFeatureMatrix.col(0).squaredNorm();
                cmponent.vector.setOnes();

                return cmponent;
            }

            auto lambdaMax = static_cast<Scalar>(-1);
            Vectori subMax;
            Vectori sub = Vectori::LinSpaced(k, 0, k - 1);
            for (Index i = 0; i < n; ++i)
            {
                const auto lambda =
                    eigenSolver
                        .maximumValueElement(deflatedFeatureMatrix(
                            Eigen::placeholders::all, sub))
                        .value;
                if (lambda > lambdaMax)
                {
                    lambdaMax = lambda;
                    subMax = sub;
                }

                sub[std::div(i, k).rem] = std::div(k + i, n).rem;
            }

            auto subDimEigenElement = eigenSolver.maximumValueElement(
                deflatedFeatureMatrix(Eigen::placeholders::all, subMax));

            Component component(n);
            component.vector(subMax) = subDimEigenElement.vector;
            component.value = subDimEigenElement.value;

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        ContiguousSupportSolver<ScalarType, EigenSolverType, ProgressBarType>::
            run(const Matrix<Scalar> &featureMatrix,
                const ComplementaryProjection<Scalar>
                    &complementaryProjectionMatrix,
                const Param &param, ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;
            using Matrix = Matrix<Scalar>;

            const Matrix deflatedFeatureMatrix =
                featureMatrix * complementaryProjectionMatrix;

            const auto &eigenSolver = param.eigenSolver;
            const auto k = param.k;
            const auto n = featureMatrix.cols();

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
                    ContiguousSupportSolver<Scalar, EigenSolver, ProgressBar>{
                        param}
                        .run(featureMatrix, complementaryProjectionMatrix));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

            components.try_emplace(
                n, eigenSolver.maximumValueElement(deflatedFeatureMatrix));

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

            for (Index k = n - 1; k > 0; --k)
            {
                components.try_emplace(
                    k,
                    ContiguousSupportSolver<Scalar, EigenSolver, ProgressBar>{
                        Param{k}}
                        .run(featureMatrix, complementaryProjectionMatrix));

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
            ContiguousSupportSolver<ScalarType, SpectraLibEigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace Sparsepc
