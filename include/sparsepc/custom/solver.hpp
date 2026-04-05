#ifndef SPARSEPC_CUSTOM_SOLVER_HPP
#define SPARSEPC_CUSTOM_SOLVER_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <limits>
#include <algorithm>
#include <future>
#include <concepts>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

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
        class CustomSolverModel final
        {
        public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            struct Param final
            {
                Param(Index kInput = static_cast<Index>(1),
                    const EigenSolver& eigenSolverInput = {},
                    Scalar zeroInput = static_cast<Scalar>(1e-6))
                    :k{ kInput }, eigenSolver{ eigenSolverInput }, zero{ zeroInput }
                {
                }

                Param(const Param&) = default;
                Param& operator=(const Param&) = default;

                Param(Param&&) = default;
                Param& operator=(Param&&) = default;

                const Index k;
                const EigenSolver eigenSolver;
                const Scalar zero;
            };

            CustomSolverModel(const Param& param = {})
                : m_Param{param}
            {
            }

            auto run(const Matrix<Scalar>& sigma) const;

            static auto runAll(const Matrix<Scalar>& sigma, const Param& param, ProgressBar* progressBar);

        private:
            const Param m_Param;
        };        

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto CustomSolverModel<ScalarType, EigenSolverType, ProgressBarType>::run(
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

            Component component(n);

            for(int i=0; i<k; ++i)
            {
                component.vector[n-1-i] = 1.0 / std::sqrt(static_cast<double>(k));
            }

            component.vector.normalize();

            component.value = (component.vector.transpose() * sigma * component.vector).value();

            return component;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto CustomSolverModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(const Matrix<Scalar>& sigma,
                                                                             const Param& param, ProgressBar* progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;

            const auto& eigenSolver = param.eigenSolver;
            const auto n = sigma.cols();

            ComponentsContainer components;

            components.emplace(n, eigenSolver.maximumValueElement(sigma));

            if (n == 1)
            {
                return components;
            }

            for (Index k = 1; k < n; ++k)
            {
                components.emplace(k, Component(n));
            }

            for (Index k = 1; k < n; ++k)
            {
                if(progressBar)
                {
                    progressBar->setValue(k);
                }

                auto& component = components.at(k);

                for(int i=0; i<k; ++i)
                {
                    component.vector[n-1-i] = 1.0 / std::sqrt(static_cast<double>(k));
                }

                component.vector.normalize();

                component.value = (component.vector.transpose() * sigma * component.vector).value();
            }

            return components;
        }

        //template<std::floating_point ScalarType>
        //using Custom = SparsePC<CustomSolverModel<ScalarType, EigenSolver<ScalarType>>>;
    }
}
#endif //SPARSEPC_CUSTOM_SOLVER_HPP
