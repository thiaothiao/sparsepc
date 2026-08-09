#pragma once

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/generic/solver.hpp>
#include <sparsepc/progress/bar.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace
{
    template <std::floating_point ScalarType>
    auto sortVector(const Sparsepc::Vector<ScalarType> &v)
    {
        using Scalar = ScalarType;
        using Index = Sparsepc::Index;

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
    auto sortMatrix(const Sparsepc::Matrix<ScalarType> &X)
    {
        using Index = Sparsepc::Index;
        using Vector = Sparsepc::Vector<ScalarType>;
        const auto n = X.cols();
        Vector v(n);
        for (Index i = 0; i < n; ++i)
        {
            v[i] = (X.transpose() * X.col(i)).cwiseAbs().sum();
        }
        return sortVector<ScalarType>(v);
    }
} // namespace

namespace Sparsepc
{
    namespace linearmodel
    {
        /**
         * @brief A wrapper class that allows to add custom C++ implementations.
         * @details Replace the current implementation with your own sparse
         * principal component implementation.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class CustomSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            /**
             * @brief CustomSolver parameter set.
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
             * @brief Constructs a new CustomSolver object with specified
             * parameters.
             * @param param The parameters used by the solver object.
             */
            CustomSolver(const Param &param = {}) : m_Param{param} {}

            /**
             * @brief Computes the principal component associated with the
             * parameters.
             * @param centeredX The featurewise centered matrix.
             * @param B The complementary projection.
             * @return The computed principal component.
             */
            auto run(const Matrix<Scalar> &centeredX,
                     const ComplementaryProjection<Scalar> &B) const;

            /**
             * @brief Computes a set of principal component candidates.
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
        auto CustomSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
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

            auto indices = sortMatrix(deflatedCenteredX);
            indices.resize(k);
            const auto subDimEigenElement = eigenSolver.maximumValueElement(
                deflatedCenteredX(Eigen::placeholders::all, indices));

            Component component(n);
            component.value = subDimEigenElement.value;
            component.vector(indices) = subDimEigenElement.vector;

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto CustomSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &centeredX,
            const ComplementaryProjection<Scalar> &B, const Param &param,
            ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;
            using Matrix = Matrix<Scalar>;

            const auto deflatedCenteredX = centeredX * B;

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
                    CustomSolver<Scalar, EigenSolver, ProgressBar>{param}.run(
                        centeredX, B));

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

            auto indices = sortMatrix(deflatedCenteredX);
            for (Index k = n - 1; k > 0; --k)
            {
                indices.resize(k);
                auto &component = components.at(k);
                const auto subDimEigenElement = eigenSolver.maximumValueElement(
                    deflatedCenteredX(Eigen::placeholders::all, indices));
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
        // using Custom = SparsePC<CustomSolver<ScalarType,
        // EigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace Sparsepc
