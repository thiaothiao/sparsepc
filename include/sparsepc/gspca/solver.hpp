#ifndef SPARSEPC_GSPCA_SOLVER_HPP
#define SPARSEPC_GSPCA_SOLVER_HPP

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
        // Greedy GSPCA implementation, see references
        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType,
             ProgressBarLike ProgressBarType = DummyProgressBar>
        class BackwardGspcaModel final
        {
        public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;
            using Component = Component<Scalar>;
            using ComponentsContainer = std::unordered_map<Index, Component>;

            struct Param final
            {
                Param(Index kInput = static_cast<Index>(1), 
                    const EigenSolver& eigenSolverInput = {},
                    Scalar zeroInput = static_cast<Scalar>(1e-8))
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

            BackwardGspcaModel(const Param& param = {}) 
                : m_Param{ param }
            {
            }

            auto run(const Matrix<Scalar>& sigma) const;

            static auto runAll(const Matrix<Scalar>& sigma, const Param& param, ProgressBar* progressBar);

        private:
            const Param m_Param;
        };

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType,
                 ProgressBarLike ProgressBarType = DummyProgressBar>
        class ForwardGspcaModel final
        {
        public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;
            using Component = Component<Scalar>;
            using ComponentsContainer = std::unordered_map<Index, Component>;

            struct Param final
            {
                Param(Index kInput = static_cast<Index>(1),
                    const EigenSolver& eigenSolverInput = {},
                    Scalar zeroInput = static_cast<Scalar>(1e-8))
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

            ForwardGspcaModel(const Param& param = {})
                : m_Param{ param }
            {
            }

            auto run(const Matrix<Scalar>& sigma) const;

            static auto runAll(const Matrix<Scalar>& sigma, const Param& param, ProgressBar* progressBar);

        private:
            const Param m_Param;
        };

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType,
                 ProgressBarLike ProgressBarType = DummyProgressBar>
        class ParallelGspcaModel final
        {
        public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;
            using Component = Component<Scalar>;
            using ComponentsContainer = std::unordered_map<Index, Component>;
            using ForwardGspcaModel = typename sparsepc::linearmodel::ForwardGspcaModel<Scalar, EigenSolver, ProgressBar>;
            using BackwardGspcaModel = typename sparsepc::linearmodel::BackwardGspcaModel<Scalar, EigenSolver, ProgressBar>;

            struct Param final
            {
                Param(Index kInput = static_cast<Index>(1),
                    const EigenSolver& eigenSolverForForwardInput = {},
                    const EigenSolver& eigenSolverForBackwardInput = {},
                    Scalar zeroInput = static_cast<Scalar>(1e-6))
                    :k{ kInput }, 
                    eigenSolverForForward{ eigenSolverForForwardInput },
                    eigenSolverForBackward{ eigenSolverForBackwardInput }, 
                    zero{ zeroInput }
                {
                }

                Param(const Param&) = default;
                Param& operator=(const Param&) = default;

                Param(Param&&) = default;
                Param& operator=(Param&&) = default;

                const Index k;
                const EigenSolver eigenSolverForForward;
                const EigenSolver eigenSolverForBackward;
                const Scalar zero;
            };

            ParallelGspcaModel(const Param& param = {})
                : m_Param{ param }
            {
            }

            auto run(const Matrix<Scalar>& sigma) const;
            
            static auto runAll(const Matrix<Scalar>& sigma, const Param& param, ProgressBar* progressBar);

        private:
            const Param m_Param;
        };

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto BackwardGspcaModel<ScalarType, EigenSolverType, ProgressBarType>::run(const Matrix<Scalar>& sigma) const
        {
            const auto k = m_Param.k;
            const auto& eigenSolver = m_Param.eigenSolver;

            const auto n = sigma.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(sigma);
            }

            Vectori choosenIndices(n);
            for (Index i = 0; i < n; ++i)
            {
                choosenIndices[i] = i;
            }

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while(true)
                {// TODO parallel computation with openmp
                    {
                        const auto candidateIndices =
                            choosenIndices.tail(static_cast<Index>(choosenIndices.size()) - j0 - 1);

                        const auto lambdaMaxSub = eigenSolver.maximumValue(sigma(candidateIndices, candidateIndices));
                        if (lambdaMaxSub > lambdaMax)
                        {
                            lambdaMax = lambdaMaxSub;
                            index = choosenIndices[j0];
                        }
                    }

                    ++j;
                    if (j < static_cast<Index>(choosenIndices.size()))
                    {
                        std::swap(choosenIndices[j0], choosenIndices[j]);
                    }
                    else 
                    {
                        break;
                    }
                }

                if (index != choosenIndices[j0])
                {// TODO use find function within tailed vector
                    for (Index j = j0 + 1; j < static_cast<Index>(choosenIndices.size()); ++j)
                    {
                        if (index == choosenIndices[j])
                        {
                            std::swap(choosenIndices[j0], choosenIndices[j]);
                            break;
                        }
                    }
                }

                ++j0;

                //std::cout << ".";

                if (static_cast<Index>(choosenIndices.size()) - j0 == k)
                {
                    break;
                }
            }
            
            auto kFoundIndices = choosenIndices.tail(k);
            auto subDimEigenElement = 
                eigenSolver.maximumValueElement(sigma(kFoundIndices, kFoundIndices));

            Component component(n);
            component.value = subDimEigenElement.value;
            component.vector(kFoundIndices) = subDimEigenElement.vector;

            return component;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto BackwardGspcaModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(const Matrix<Scalar>& sigma,
            const Param& param, ProgressBar* progressBar)
        {
            const auto& eigenSolver = param.eigenSolver;
            const auto n = sigma.cols();

            ComponentsContainer components;

            components.emplace(n, eigenSolver.maximumValueElement(sigma));

            if (n == static_cast<Index>(1))
            {
                return components;
            }

            Vectori choosenIndices(n);
            for (Index i = 0; i < n; ++i)
            {
                choosenIndices[i] = i;
            }

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while (true)
                {// TODO parallel computation with openmp
                    {
                        const auto candidateIndices =
                            choosenIndices.tail(static_cast<Index>(choosenIndices.size())-j0-1);

                        const auto lambdaMaxSub = eigenSolver.maximumValue(sigma(candidateIndices, candidateIndices));
                        if (lambdaMaxSub > lambdaMax)
                        {
                            lambdaMax = lambdaMaxSub;
                            index = choosenIndices[j0];
                        }
                    }

                    ++j;
                    if (j < static_cast<Index>(choosenIndices.size()))
                    {
                        std::swap(choosenIndices[j0], choosenIndices[j]);
                    }
                    else
                    {
                        break;
                    }
                }

                if (index != choosenIndices[j0])
                {// TODO use find function within tailed vector
                    for (Index j = j0 + 1; j < static_cast<Index>(choosenIndices.size()); ++j)
                    {
                        if (index == choosenIndices[j])
                        {
                            std::swap(choosenIndices[j0], choosenIndices[j]);
                            break;
                        }
                    }
                }

                ++j0;

                const auto k = static_cast<Index>(choosenIndices.size()) - j0;
                const auto kFoundIndices = choosenIndices.tail(k);

                auto subDimEigenElement = eigenSolver.maximumValueElement(sigma(kFoundIndices, kFoundIndices));

                auto iter = components.emplace(k, Component(n));
                iter->first.value = subDimEigenElement.value;
                iter->first.vector(kFoundIndices) = subDimEigenElement.vector;

                if(progressBar)
                {
                    progressBar->setValue(j0);
                }

                if (k == 1)
                {
                    break;
                }
            }

            return components;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto ForwardGspcaModel<ScalarType, EigenSolverType, ProgressBarType>::run(const Matrix<Scalar>& sigma) const
        {
            const auto k = m_Param.k;
            const auto& eigenSolver = m_Param.eigenSolver;

            const auto n = sigma.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(sigma);
            }

            Vectori reserveIndices(n);
            for (Index i = 0; i < n; ++i)
            {
                reserveIndices[i] = i;
            }

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while (true)
                {// TODO parallel computation with openmp
                    {
                        const auto candidateIndices = reserveIndices.head(j0+1);
                        const auto lambdaMaxSub = eigenSolver.maximumValue(sigma(candidateIndices, candidateIndices));
                        if (lambdaMaxSub > lambdaMax)
                        {
                            lambdaMax = lambdaMaxSub;
                            index = reserveIndices[j0];
                        }
                    }

                    ++j;
                    if (j < static_cast<Index>(reserveIndices.size()))
                    {
                        std::swap(reserveIndices[j0], reserveIndices[j]);
                    }
                    else
                    {
                        break;
                    }
                }

                if (index != reserveIndices[j0])
                {// TODO use find function within tailed vector
                    for (Index j = j0 + 1; j < static_cast<Index>(reserveIndices.size()); ++j)
                    {
                        if (index == reserveIndices[j])
                        {
                            std::swap(reserveIndices[j0], reserveIndices[j]);
                            break;
                        }
                    }
                }

                ++j0;
                //std::cout << ".";
                if (j0 == k)
                {
                    break;
                }
            }

            const auto kFoundIndices = reserveIndices.head(k);
            auto subDimEigenElement = eigenSolver.maximumValueElement(sigma(kFoundIndices, kFoundIndices));

            Component component(n);
            component.value = subDimEigenElement.value;
            component.vector(kFoundIndices) = subDimEigenElement.vector;

            return component;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto ForwardGspcaModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(const Matrix<Scalar>& sigma,
            const Param& param, ProgressBar* progressBar)
        {
            const auto& eigenSolver = param.eigenSolver;
            const auto n = sigma.cols();

            ComponentsContainer components;

            components.emplace(n, eigenSolver.maximumValueElement(sigma));

            if (n == static_cast<Index>(1))
            {
                return components;
            }

            Vectori reserveIndices(n);
            for (Index i = 0; i < n; ++i)
            {
                reserveIndices[i] = i;
            }

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while (true)
                {// TODO parallel computation with openmp
                    {
                        const auto candidateIndices = reserveIndices.head(j0 + 1);
                        const auto lambdaMaxSub = eigenSolver.maximumValue(sigma(candidateIndices, candidateIndices));
                        if (lambdaMaxSub > lambdaMax)
                        {
                            lambdaMax = lambdaMaxSub;
                            index = reserveIndices[j0];
                        }
                    }

                    ++j;
                    if (j < static_cast<Index>(reserveIndices.size()))
                    {
                        std::swap(reserveIndices[j0], reserveIndices[j]);
                    }
                    else
                    {
                        break;
                    }
                }

                if (index != reserveIndices[j0])
                {// TODO use find function within tailed vector
                    for (Index j = j0 + 1; j < static_cast<Index>(reserveIndices.size()); ++j)
                    {
                        if (index == reserveIndices[j])
                        {
                            std::swap(reserveIndices[j0], reserveIndices[j]);
                            break;
                        }
                    }
                }

                ++j0;

                const auto k = j0;
                const auto kFoundIndices = reserveIndices.head(k);
                auto subDimEigenElement = eigenSolver.maximumValueElement(sigma(kFoundIndices, kFoundIndices));

                auto iter = components.emplace(k, Component(n));
                iter->first.value = subDimEigenElement.value;
                iter->first.vector(kFoundIndices) = subDimEigenElement.vector;

                if(progressBar)
                {
                    progressBar->setValue(j0);
                }

                if (k == n - 1)
                {
                    break;
                }
            }

            return components;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto ParallelGspcaModel<ScalarType, EigenSolverType, ProgressBarType>::run(const Matrix<Scalar>& sigma) const
        {
            const auto k = m_Param.k;
            const auto& forwardSolver = m_Param.eigenSolverForForward;
            const auto& backwardSolver = m_Param.eigenSolverForBackward;
            const auto zero = m_Param.zero;

            const auto n = sigma.cols();

            if (k >= n || k < static_cast<Index>(0))
            {
                return forwardSolver.maximumValueElement(sigma);
            }

            const typename ForwardGspcaModel::Param forwardParam{ k, forwardSolver, zero};
            const ForwardGspcaModel forwardGspca{ forwardParam };

            const typename BackwardGspcaModel::Param backwardParam{ k, backwardSolver, zero };
            const BackwardGspcaModel backwardGspca{ backwardParam};

            auto forwardSolutionFuture =
                std::async(std::launch::async, &ForwardGspcaModel::run, &forwardGspca, sigma);

            Component backwardSolutionSparseEigenElement = backwardGspca.run(sigma);

            Component forwardSolutionSparseEigenElement = forwardSolutionFuture.get();

            if (forwardSolutionSparseEigenElement.value >= backwardSolutionSparseEigenElement.value)
            {
                return forwardSolutionSparseEigenElement;
            }
            else
            {
                return backwardSolutionSparseEigenElement;
            }
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto ParallelGspcaModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(const Matrix<Scalar>& sigma,
            const Param& param, ProgressBar* progressBar)
        {
            const auto k = param.k;
            const auto& forwardSolver = param.eigenSolverForForward;
            const auto& backwardSolver = param.eigenSolverForBackward;
            const auto zero = param.zero;

            const typename ForwardGspcaModel::Param forwardParam{ k, forwardSolver, zero };
            const typename BackwardGspcaModel::Param backwardParam{ k, backwardSolver, zero };

            auto forwardSolutionFuture =
                std::async(std::launch::async, &ForwardGspcaModel::runAll, sigma, forwardParam, progressBar);

            ComponentsContainer backwardSolutionSparseEigenElement
                = BackwardGspcaModel::runAll(sigma, backwardParam, progressBar);

            ComponentsContainer forwardSolutionSparseEigenElement = forwardSolutionFuture.get();

            const auto n = sigma.cols();

            for (Index j = 1; j <= n; ++j)
            {
                if (backwardSolutionSparseEigenElement.at(j).value < forwardSolutionSparseEigenElement.at(j).value)
                {
                    backwardSolutionSparseEigenElement.at(j) = std::move(forwardSolutionSparseEigenElement.at(j));
                }
            }

            return backwardSolutionSparseEigenElement;
        }

        template<std::floating_point ScalarType>
        using BackwardGspca = SparsePC<BackwardGspcaModel<ScalarType, EigenSolver<ScalarType>>>;

        template<std::floating_point ScalarType>
        using ForwardGspca = SparsePC<ForwardGspcaModel<ScalarType, EigenSolver<ScalarType>>>;

        template<std::floating_point ScalarType>
        using ParallelGspca = SparsePC<ParallelGspcaModel<ScalarType, EigenSolver<ScalarType>>>;
    }
}
#endif //SPARSEPC_GSPCA_SOLVER_HPP
