#pragma once

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/generic/solver.hpp>
#include <sparsepc/progress/bar.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace
{
    template <std::floating_point ScalarType>
    auto sortVector(const sparsepc::Vector<ScalarType> &v)
    {
        using Scalar = ScalarType;
        using Index = sparsepc::Index;

        std::vector<std::pair<Index, Scalar>> worker;
        worker.reserve(v.size());
        for (Index i = 0; i < v.size(); ++i)
        {
            worker.emplace_back(i, v[i]);
        }

        std::sort(worker.begin(), worker.end(),
                  [](const std::pair<Index, Scalar> &lhs,
                     const std::pair<Index, Scalar> &rhs) {
                      return lhs.second > rhs.second;
                  });

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
        return sortVector<ScalarType>(sigma.cwiseAbs().colwise().sum().eval());
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

            CustomSolverModel(const Param &param = {}) : m_Param{param} {}

            auto run(const Matrix<Scalar> &sigma) const;

            static auto run(const Matrix<Scalar> &sigma, const Param &param,
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
        CustomSolverModel<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &sigma, const Param &param,
            ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;

            const auto &eigenSolver = param.eigenSolver;
            const auto k = param.k;
            const auto n = sigma.cols();

            ComponentsContainer components;

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

                components.emplace(
                    std::min(k, n),
                    CustomSolverModel<Scalar, EigenSolver, ProgressBar>{param}
                        .run(sigma));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

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

            auto indices = sortMatrix(sigma);
            for (Index k = n - 1; k > 1; --k)
            {
                indices.resize(k);
                auto &component = components.at(k);
                const auto subDimEigenElement =
                    eigenSolver.maximumValueElement(sigma(indices, indices));
                component.value = subDimEigenElement.value;
                component.vector(indices) = subDimEigenElement.vector;

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
        // template<std::floating_point ScalarType>
        // using Custom = SparsePC<CustomSolverModel<ScalarType,
        // EigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace sparsepc
