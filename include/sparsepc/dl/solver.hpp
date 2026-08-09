#pragma once

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/generic/solver.hpp>
#include <sparsepc/progress/bar.hpp>
#include <sparsepc/utils/matrix.hpp>

using ComputeSparseEigenVector = void (*)(const double *, int, int, double *);

namespace Sparsepc
{
    namespace linearmodel
    {
        /**
         * @brief A wrapper class for the ComputeSparseEigenVector function from
         * an addon.
         * @details It calls the function exposed by the considered dynamic
         * library.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class DynamicLibSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            /**
             * @brief DynamicLibSolver parameter set.
             * @details Store parameters needed for the computations and the
             * addon interface.
             */
            struct Param final
            {
                Param(ComputeSparseEigenVector computeSparseEigenVectorInput =
                          nullptr,
                      Index kInput = static_cast<Index>(-1),
                      const EigenSolver &eigenSolverInput = {},
                      Scalar zeroInput = static_cast<Scalar>(1e-6))
                    : computeSparseEigenVector{computeSparseEigenVectorInput},
                      k{kInput}, eigenSolver{eigenSolverInput}, zero{zeroInput}
                {
                }

                Param(const Param &) = default;
                Param &operator=(const Param &) = default;

                Param(Param &&) = default;
                Param &operator=(Param &&) = default;

                const Index k;
                const EigenSolver eigenSolver;
                const Scalar zero;
                const ComputeSparseEigenVector computeSparseEigenVector;
            };

            /**
             * @brief Constructs a new DynamicLibSolver object with the
             * specified parameters.
             * @param param The parameters used by the solver object.
             */
            DynamicLibSolver(const Param &param = {}) : m_Param{param} {}

            /**
             * @brief Computes the principal component associated with the
             * parameters by calling the addon interface.
             * @param centeredX The featurewise centered matrix.
             * @param B The complementary projection.
             * @return The computed principal component.
             */
            auto run(const Matrix<Scalar> &centeredX,
                     const ComplementaryProjection<Scalar> &B) const;

            /**
             * @brief Computes a set of principal component candidates by
             * several calls to the addon function.
             * @param centeredX The featurewise centered matrix.
             * @param B The complementary projection.
             * @param param The parameters to be used.
             * @param progressBar The computation progress reporter.
             * @return The computed candidates.
             */
            static auto run(const Matrix<Scalar> &centeredX,
                            const ComplementaryProjection<Scalar> &B,
                            const Param &param, ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        DynamicLibSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &centeredX,
            const ComplementaryProjection<Scalar> &B) const
        {
            using Component = Component<Scalar>;
            using Matrix = Matrix<Scalar>;

            const Matrix deflatedCenteredX = centeredX * B;

            const auto k = m_Param.k;
            const auto &eigenSolver = m_Param.eigenSolver;

            const auto n = deflatedCenteredX.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(deflatedCenteredX);
            }

            if (static_cast<Index>(1) == n)
            {
                Component cmponent(n);
                cmponent.value = deflatedCenteredX.col(0).squaredNorm();
                cmponent.vector.setOnes();

                return cmponent;
            }

            Component component(n);
            if (m_Param.computeSparseEigenVector)
            {
                m_Param.computeSparseEigenVector(deflatedCenteredX.data(), n, k,
                                                 component.vector.data());

                component.vector.normalize();

                component.value =
                    (deflatedCenteredX * component.vector).squaredNorm();
            }

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        DynamicLibSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &centeredX,
            const ComplementaryProjection<Scalar> &B, const Param &param,
            ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;
            using Matrix = Matrix<Scalar>;

            const Matrix deflatedCenteredX = centeredX * B;

            const auto &eigenSolver = param.eigenSolver;
            const auto k = param.k;
            const auto n = deflatedCenteredX.cols();

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
                    DynamicLibSolver<Scalar, EigenSolver, ProgressBar>{param}
                        .run(centeredX, B));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

            components.try_emplace(
                n, eigenSolver.maximumValueElement(deflatedCenteredX));

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

            if (param.computeSparseEigenVector)
            {
                for (Index k = 1; k < n; ++k)
                {
                    auto &component = components.at(k);
                    param.computeSparseEigenVector(deflatedCenteredX.data(), n,
                                                   k, component.vector.data());
                    component.vector.normalize();
                    component.value =
                        (deflatedCenteredX * component.vector).squaredNorm();

                    if (progressBar)
                    {
                        progressBar->setValue(k + 1);

                        progressBar->processEvents();

                        if (progressBar->wasCanceled())
                        {
                            return components;
                        }
                    }
                }
            }

            return components;
        }

        // template<std::floating_point ScalarType>
        // using User = SparsePC<DynamicLibSolver<ScalarType,
        // EigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace Sparsepc
