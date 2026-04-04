#ifndef SPARSEPC_DCA_SOLVER_HPP
#define SPARSEPC_DCA_SOLVER_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <limits>
#include <algorithm>
#include <future>
#include <concepts>

#include "sparsepc/utils/matrix.hpp"
#include "sparsepc/eigen/solver.hpp"
#include "sparsepc/generic/solver.hpp"
#include "sparsepc/progress/bar.hpp"

namespace sparsepc
{
    namespace linearmodel
    {
        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType,
             ProgressBarLike ProgressBarType = DummyProgressBar>
        class UserModel final
        {
        public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            struct Param final
            {
                Param(Index kInput = static_cast<Index>(1),
                    const EigenSolver& eigenSolverInput = {},
                    Scalar tInput = static_cast<Scalar>(1000000),
                    Scalar toleranceInput = static_cast<Scalar>(1e-4),
                    unsigned int maximumNumberOfIterationsInput = 10000U,
                    Scalar zeroInput = static_cast<Scalar>(1e-6))
                    :k{ kInput }, eigenSolver{ eigenSolverInput }, 
                    t{ tInput }, tolerance{ toleranceInput },
                    maximumNumberOfIterations{ maximumNumberOfIterationsInput },
                    zero{ zeroInput }
                {
                }

                Param(const Param&) = default;
                Param& operator=(const Param&) = default;

                Param(Param&&) = default;
                Param& operator=(Param&&) = default;

                const Index k;
                const EigenSolver eigenSolver;
                const Scalar t;
                const Scalar tolerance;
                const unsigned int maximumNumberOfIterations;
                const Scalar zero;
            };

            UserModel(const Param& param = {})
                : m_Param{param}
            {
            }

            auto run(const Matrix<Scalar>& sigma) const;

            static auto runAll(const Matrix<Scalar>& sigma, const Param& param, ProgressBar* progressBar);

        private:
            const Param m_Param;
        };

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto UserModel<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar>& sigma) const
        {
            using Component = Component<Scalar>;

            const auto k = m_Param.k;
            const auto& eigenSolver = m_Param.eigenSolver;

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

            Component component;// = eigenSolver.maximumValueElement(sigma);


            component.vector;//= ...;Solve sparse eigen with cardinality k

            component.vector.normalize();

            component.value = (component.vector.transpose() * sigma * component.vector).value();

            return component;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto UserModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(const Matrix<Scalar>& sigma,
            const Param& param, ProgressBar* progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;

            const auto& eigenSolver = param.eigenSolver;
            const auto n = sigma.cols();

            ComponentsContainer components;

            const auto& component = components.emplace(n, eigenSolver.maximumValueElement(sigma)).first->second;

            if (n == 1)
            {
                return components;
            }

            for (Index k = 1; k < n; ++k)
            {
                components.emplace(k, Component{});
            }

            for (Index k = 1; k < n; ++k)
            {
                components.at(k) = UserModel<Scalar, EigenSolver, ProgressBar>{
                    Param{ k, param.eigenSolver, param.t, param.tolerance, param.maximumNumberOfIterations, param.zero }
                }.run(sigma, component);
            }

            return components;
        }

        template<std::floating_point ScalarType>
        using User = SparsePC<UserModel<ScalarType, EigenSolver<ScalarType>>>;
    }
}
#endif //SPARSEPC_DCA_SOLVER_HPP
