#ifndef SPARSEPC_CUSTOM_SOLVER_HPP
#define SPARSEPC_CUSTOM_SOLVER_HPP

#include "sparsepc/eigen/solver.hpp"
#include "sparsepc/generic/solver.hpp"
#include "sparsepc/progress/bar.hpp"
#include "sparsepc/utils/matrix.hpp"

namespace
{
    template <std::floating_point ScalarType>
    auto sortVector(const sparsepc::Vector<ScalarType> &v)
    {
        using Scalar = ScalarType;
        using Index = sparsepc::Index;
        auto comparePairsLambda = [](const std::pair<Index, Scalar> &lhs,
                                     const std::pair<Index, Scalar> &rhs) {
            return lhs.second > rhs.second;
        };
        std::set<std::pair<Index, Scalar>, decltype(comparePairsLambda)> worker(
            comparePairsLambda);
        for (Index i = 0; i < v.size(); ++i)
        {
            worker.insert({i, v[i]});
        }
        std::vector<Index> indices;
        indices.reserve(v.size());
        for (const auto &w : worker)
        {
            indices.push_back(w.first);
        }
        return indices;
    }

    template <std::floating_point ScalarType>
    auto sortMatrix(const sparsepc::Matrix<ScalarType> &sigma)
    {
        const sparsepc::Vector<ScalarType> v =
            sigma.cwiseAbs().colwise().sum().eval();
        return sortVector(v);
    }
} // namespace

namespace sparsepc
{
    namespace linearmodel
    {
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
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

            CustomSolverModel(const Param &param = {}) : m_Param{param} {}

            auto run(const Matrix<Scalar> &sigma) const;

            static auto runAll(const Matrix<Scalar> &sigma, const Param &param,
                               ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        CustomSolverModel<ScalarType, EigenSolverType, ProgressBarType>::run(
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

            auto indices = sortMatrix(sigma);
            indices.resize(k);
            const auto subDimEigenElement =
                eigenSolver.maximumValueElement(sigma(indices, indices));

            Component component(n);
            component.value = subDimEigenElement.value;
            component.vector(indices) = subDimEigenElement.vector;

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        CustomSolverModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(
            const Matrix<Scalar> &sigma, const Param &param,
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

            for (Index k = 1; k < n; ++k)
            {
                components.emplace(k, Component(n));
            }

            const auto indices = sortMatrix(sigma);
            std::vector<Index> kIndices;
            kIndices.reserve(n);
            for (Index k = 1; k < n; ++k)
            {
                if (progressBar)
                {
                    progressBar->setValue(k);
                }

                kIndices.push_back(indices[k - 1]);

                auto &component = components.at(k);
                const auto subDimEigenElement =
                    eigenSolver.maximumValueElement(sigma(kIndices, kIndices));
                component.value = subDimEigenElement.value;
                component.vector(kIndices) = subDimEigenElement.vector;
            }

            return components;
        }
        // template<std::floating_point ScalarType>
        // using Custom = SparsePC<CustomSolverModel<ScalarType,
        // EigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace sparsepc
#endif // SPARSEPC_CUSTOM_SOLVER_HPP
