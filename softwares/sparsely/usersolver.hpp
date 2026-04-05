#ifndef SPARSEPC_USER_SOLVER_HPP
#define SPARSEPC_USER_SOLVER_HPP

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

#include "spcaloader.hpp"

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
        class DllSolverModel final
        {
        public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            struct Param final
            {
                Param(const plugin::SpcaLoader* loaderInput = nullptr,
                      Index kInput = static_cast<Index>(1),
                    const EigenSolver& eigenSolverInput = {},
                    Scalar zeroInput = static_cast<Scalar>(1e-6))
                    :loader{loaderInput}, k{ kInput },
                    eigenSolver{ eigenSolverInput }, zero{ zeroInput }
                {
                }

                Param(const Param&) = default;
                Param& operator=(const Param&) = delete;

                Param(Param&&) = delete;
                Param& operator=(Param&&) = delete;

                const Index k;
                const EigenSolver eigenSolver;
                const Scalar zero;
                const plugin::SpcaLoader* loader;
            };

            DllSolverModel(const Param& param = {})
                : m_Param{param}
            {
            }

            auto run(const Matrix<Scalar>& sigma) const;

            static auto runAll(const Matrix<Scalar>& sigma, const Param& param, ProgressBar* progressBar);

        private:
            const Param m_Param;
        };        

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto DllSolverModel<ScalarType, EigenSolverType, ProgressBarType>::run(
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
            if(m_Param.loader)
            try
            {
                const plugin::SPCA& spca = m_Param.loader->getSpca();

                spca.solve(sigma.data(), n, k, component.vector.data());

                component.vector.normalize();

                component.value = (component.vector.transpose() * sigma * component.vector).value();

                std::cout << "\n Job done.\n";
            }
            catch (std::out_of_range const&)
            {
                std::cout << "\n Wrong.\n";
            }

            return component;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto DllSolverModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(const Matrix<Scalar>& sigma,
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
                components.emplace(k, Component{});
            }

            if(param.loader)
            try
            {
                const plugin::SPCA& spca = param.loader->getSpca();

                for (Index k = 1; k < n; ++k)
                {
                    if(progressBar)
                    {
                        progressBar->setValue(k);
                    }

                    auto& component = components.at(k);

                    spca.solve(sigma.data(), n, k, component.vector.data());

                    component.vector.normalize();

                    component.value = (component.vector.transpose() * sigma * component.vector).value();
                }

                std::cout << "\n Job done.\n";
            }
            catch (std::out_of_range const&)
            {
                std::cout << "\n Wrong.\n";
            }

            return components;
        }

        //template<std::floating_point ScalarType>
        //using User = SparsePC<DllSolverModel<ScalarType, EigenSolver<ScalarType>>>;
    }
}
#endif //SPARSEPC_USER_SOLVER_HPP
