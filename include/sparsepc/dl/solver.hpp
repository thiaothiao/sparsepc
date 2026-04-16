#ifndef SPARSEPC_DL_SOLVER_HPP
#define SPARSEPC_DL_SOLVER_HPP

#include "sparsepc/eigen/solver.hpp"
#include "sparsepc/generic/solver.hpp"
#include "sparsepc/progress/bar.hpp"
#include "sparsepc/utils/matrix.hpp"

using ComputeSparseEigenVector = void (*)(const double *, int, int, double *);

namespace sparsepc
{
    namespace linearmodel
    {
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class DynamicLibSolverModel final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            struct Param final
            {
                Param(ComputeSparseEigenVector computeSparseEigenVectorInput =
                          nullptr,
                      Index kInput = static_cast<Index>(1),
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

            DynamicLibSolverModel(const Param &param = {}) : m_Param{param} {}

            auto run(const Matrix<Scalar> &sigma) const;

            static auto runAll(const Matrix<Scalar> &sigma, const Param &param,
                               ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto DynamicLibSolverModel<ScalarType, EigenSolverType,
                                   ProgressBarType>::run(const Matrix<Scalar>
                                                             &sigma) const
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

            Component component(n);
            if (m_Param.computeSparseEigenVector)
            {
                m_Param.computeSparseEigenVector(sigma.data(), n, k,
                                                 component.vector.data());

                component.vector.normalize();

                component.value =
                    (component.vector.transpose() * sigma * component.vector)
                        .value();
            }

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        DynamicLibSolverModel<ScalarType, EigenSolverType, ProgressBarType>::
            runAll(const Matrix<Scalar> &sigma, const Param &param,
                   ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;

            const auto &eigenSolver = param.eigenSolver;
            const auto n = sigma.cols();

            ComponentsContainer components;

            components.emplace(n, eigenSolver.maximumValueElement(sigma));

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
                components.emplace(k, Component(n));
            }

            if (param.computeSparseEigenVector)
            {
                for (Index k = 1; k < n; ++k)
                {
                    auto &component = components.at(k);
                    param.computeSparseEigenVector(sigma.data(), n, k,
                                                   component.vector.data());
                    component.vector.normalize();
                    component.value = (component.vector.transpose() * sigma *
                                       component.vector)
                                          .value();

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
        // using User = SparsePC<DynamicLibSolverModel<ScalarType,
        // EigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace sparsepc
#endif // SPARSEPC_DL_SOLVER_HPP
