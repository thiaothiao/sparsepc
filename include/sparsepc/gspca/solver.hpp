#pragma once

#include <algorithm>
#include <concepts>
#include <future>

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/generic/solver.hpp>
#include <sparsepc/progress/bar.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace Sparsepc
{
    namespace linearmodel
    {
        /**
         * @brief Backward Gspca method class.
         * @details It implements the backward version of the greedy Gspca
         * method.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class BackwardGspcaSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            /**
             * @brief BackwardGspcaSolver parameter set.
             * @details Store parameters needed for the computations.
             */
            struct Param final
            {
                Param(Index kInput = static_cast<Index>(-1),
                      const EigenSolver &eigenSolverInput = {},
                      Scalar zeroInput = static_cast<Scalar>(1e-8))
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
             * @brief Constructs a new BackwardGspcaSolver object with the
             * specified parameters.
             * @param param The parameters used by the solver object.
             */
            BackwardGspcaSolver(const Param &param = {}) : m_Param{param} {}

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

        /**
         * @brief Forward Gspca method class.
         * @details It implements the forward version of the greedy Gspca
         * method.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class ForwardGspcaSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            /**
             * @brief ForwardGspcaSolver parameter set.
             * @details Store parameters needed for the computations.
             */
            struct Param final
            {
                Param(Index kInput = static_cast<Index>(-1),
                      const EigenSolver &eigenSolverInput = {},
                      Scalar zeroInput = static_cast<Scalar>(1e-8))
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
             * @brief Constructs a new ForwardGspcaSolver object with the
             * specified parameters.
             * @param param The parameters used by the solver object.
             */
            ForwardGspcaSolver(const Param &param = {}) : m_Param{param} {}

            /**
             * @brief Computes the principal component associated with the
             * parameters.
             * @param centeredX The featurewise centered matrix.
             * @param B The complementary projection.
             * @return The computed principal component.
             */
            Component<Scalar>
            run(const Matrix<Scalar> &centeredX,
                const ComplementaryProjection<Scalar> &B) const;

            /**
             * @brief Computes a set of principal component candidates.
             * @param centeredX The featurewise centered matrix.
             * @param B The complementary projection.
             * @param param The parameters to be used.
             * @param progressBar The computation progress reporter.
             * @return The computed candidates.
             */
            static ComponentsContainer<Component<Scalar>>
            run(const Matrix<Scalar> &centeredX,
                const ComplementaryProjection<Scalar> &B, const Param &param,
                ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        /**
         * @brief Parallel Gspca method class.
         * @details It implements the forward and backward versions of the
         * greedy Gspca method. Both methods run in parallel and the best
         * component is then selected.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class ParallelGspcaSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;
            using ForwardGspcaSolver =
                typename Sparsepc::linearmodel::ForwardGspcaSolver<
                    Scalar, EigenSolver, ProgressBar>;
            using BackwardGspcaSolver =
                typename Sparsepc::linearmodel::BackwardGspcaSolver<
                    Scalar, EigenSolver, ProgressBar>;

            /**
             * @brief ParallelGspcaSolver parameter set.
             * @details Store parameters needed for the computations.
             */
            struct Param final
            {
                Param(Index kInput = static_cast<Index>(-1),
                      const EigenSolver &eigenSolverForForwardInput = {},
                      const EigenSolver &eigenSolverForBackwardInput = {},
                      Scalar zeroInput = static_cast<Scalar>(1e-6))
                    : k{kInput},
                      eigenSolverForForward{eigenSolverForForwardInput},
                      eigenSolverForBackward{eigenSolverForBackwardInput},
                      zero{zeroInput}
                {
                }

                Param(const Param &) = default;
                Param &operator=(const Param &) = default;

                Param(Param &&) = default;
                Param &operator=(Param &&) = default;

                const Index k;
                const EigenSolver eigenSolverForForward;
                const EigenSolver eigenSolverForBackward;
                const Scalar zero;
            };

            /**
             * @brief Constructs a new ParallelGspcaSolver object with the
             * specified parameters.
             * @param param The parameters used by the solver object.
             */
            ParallelGspcaSolver(const Param &param = {}) : m_Param{param} {}

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
        auto
        BackwardGspcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
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

            Vectori choosenIndices = Vectori::LinSpaced(n, 0, n - 1);

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while (true)
                { // TODO parallel computation with openmp
                    {
                        const auto candidateIndices = choosenIndices.tail(
                            static_cast<Index>(choosenIndices.size()) - j0 - 1);

                        const auto lambdaMaxSub =
                            eigenSolver.maximumValue(deflatedCenteredX(
                                Eigen::placeholders::all, candidateIndices));
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
                { // TODO use find function within tailed vector
                    for (Index i = j0 + 1;
                         i < static_cast<Index>(choosenIndices.size()); ++i)
                    {
                        if (index == choosenIndices[i])
                        {
                            std::swap(choosenIndices[j0], choosenIndices[i]);
                            break;
                        }
                    }
                }

                ++j0;

                if (static_cast<Index>(choosenIndices.size()) - j0 == k)
                {
                    break;
                }
            }

            auto kFoundIndices = choosenIndices.tail(k);
            auto subDimEigenElement = eigenSolver.maximumValueElement(
                deflatedCenteredX(Eigen::placeholders::all, kFoundIndices));

            Component component(n);
            component.value = subDimEigenElement.value;
            component.vector(kFoundIndices) = subDimEigenElement.vector;

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        BackwardGspcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &centeredX,
            const ComplementaryProjection<Scalar> &B, const Param &param,
            ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;
            using Matrix = Matrix<Scalar>;

            const auto delatedCenteredX = centeredX * B;

            const auto &eigenSolver = param.eigenSolver;
            const auto k = param.k;
            const auto n = delatedCenteredX.cols();

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
                    BackwardGspcaSolver<Scalar, EigenSolver, ProgressBar>{param}
                        .run(centeredX, B));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

            components.try_emplace(
                n, eigenSolver.maximumValueElement(delatedCenteredX));

            if (n == static_cast<Index>(1))
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

            Vectori choosenIndices = Vectori::LinSpaced(n, 0, n - 1);

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while (true)
                { // TODO parallel computation with openmp
                    {
                        const auto candidateIndices = choosenIndices.tail(
                            static_cast<Index>(choosenIndices.size()) - j0 - 1);

                        const auto lambdaMaxSub =
                            eigenSolver.maximumValue(delatedCenteredX(
                                Eigen::placeholders::all, candidateIndices));
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
                { // TODO use find function within tailed vector
                    for (Index i = j0 + 1;
                         i < static_cast<Index>(choosenIndices.size()); ++i)
                    {
                        if (index == choosenIndices[i])
                        {
                            std::swap(choosenIndices[j0], choosenIndices[i]);
                            break;
                        }
                    }
                }

                ++j0;

                const auto kCurrent =
                    static_cast<Index>(choosenIndices.size()) - j0;
                const auto kFoundIndices = choosenIndices.tail(kCurrent);

                auto subDimEigenElement = eigenSolver.maximumValueElement(
                    delatedCenteredX(Eigen::placeholders::all, kFoundIndices));

                auto &component =
                    components.try_emplace(kCurrent, Component(n)).first->second;
                component.value = subDimEigenElement.value;
                component.vector(kFoundIndices) = subDimEigenElement.vector;

                if (progressBar)
                {
                    progressBar->setValue(static_cast<int>(j0) + 1);

                    progressBar->processEvents();

                    if (progressBar->wasCanceled())
                    {
                        return components;
                    }
                }

                if (kCurrent == 1)
                {
                    break;
                }
            }

            return components;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        Component<typename ForwardGspcaSolver<ScalarType, EigenSolverType,
                                              ProgressBarType>::Scalar>
        ForwardGspcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
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

            Vectori reserveIndices = Vectori::LinSpaced(n, 0, n - 1);

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while (true)
                { // TODO parallel computation with openmp
                    {
                        const auto candidateIndices =
                            reserveIndices.head(j0 + 1);
                        const auto lambdaMaxSub =
                            eigenSolver.maximumValue(deflatedCenteredX(
                                Eigen::placeholders::all, candidateIndices));
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
                { // TODO use find function within tailed vector
                    for (Index i = j0 + 1;
                         i < static_cast<Index>(reserveIndices.size()); ++i)
                    {
                        if (index == reserveIndices[i])
                        {
                            std::swap(reserveIndices[j0], reserveIndices[i]);
                            break;
                        }
                    }
                }

                ++j0;

                if (j0 == k)
                {
                    break;
                }
            }

            const auto kFoundIndices = reserveIndices.head(k);
            auto subDimEigenElement = eigenSolver.maximumValueElement(
                deflatedCenteredX(Eigen::placeholders::all, kFoundIndices));

            Component component(n);
            component.value = subDimEigenElement.value;
            component.vector(kFoundIndices) = subDimEigenElement.vector;

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        ComponentsContainer<Component<typename ForwardGspcaSolver<
            ScalarType, EigenSolverType, ProgressBarType>::Scalar>>
        ForwardGspcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
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
                    ForwardGspcaSolver<Scalar, EigenSolver, ProgressBar>{param}
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

            if (n == static_cast<Index>(1))
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

            Vectori reserveIndices = Vectori::LinSpaced(n, 0, n - 1);

            auto j0 = static_cast<Index>(0);
            while (true)
            {
                auto lambdaMax = static_cast<Scalar>(0);
                auto index = static_cast<Index>(-1);
                Index j = j0;
                while (true)
                { // TODO parallel computation with openmp
                    {
                        const auto candidateIndices =
                            reserveIndices.head(j0 + 1);
                        const auto lambdaMaxSub =
                            eigenSolver.maximumValue(deflatedCenteredX(
                                Eigen::placeholders::all, candidateIndices));
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
                { // TODO use find function within tailed vector
                    for (Index i = j0 + 1;
                         i < static_cast<Index>(reserveIndices.size()); ++i)
                    {
                        if (index == reserveIndices[i])
                        {
                            std::swap(reserveIndices[j0], reserveIndices[i]);
                            break;
                        }
                    }
                }

                ++j0;

                const auto kCurrent = j0;
                const auto kFoundIndices = reserveIndices.head(kCurrent);
                auto subDimEigenElement = eigenSolver.maximumValueElement(
                    deflatedCenteredX(Eigen::placeholders::all, kFoundIndices));

                auto &component =
                    components.try_emplace(kCurrent, Component(n)).first->second;
                component.value = subDimEigenElement.value;
                component.vector(kFoundIndices) = subDimEigenElement.vector;

                if (progressBar)
                {
                    progressBar->setValue(static_cast<int>(j0) + 1);

                    progressBar->processEvents();

                    if (progressBar->wasCanceled())
                    {
                        return components;
                    }
                }

                if (kCurrent == n - 1)
                {
                    break;
                }
            }

            return components;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        ParallelGspcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &centeredX,
            const ComplementaryProjection<Scalar> &B) const
        {
            using Component = Component<Scalar>;
            using Matrix = Matrix<Scalar>;
            using ComplementaryProjection = ComplementaryProjection<Scalar>;

            const Matrix deflatedCenteredX = centeredX * B;

            const auto k = m_Param.k;
            const auto &forwardSolver = m_Param.eigenSolverForForward;
            const auto &backwardSolver = m_Param.eigenSolverForBackward;
            const auto zero = m_Param.zero;

            const auto n = deflatedCenteredX.cols();

            if (k >= n || k < static_cast<Index>(0))
            {
                return forwardSolver.maximumValueElement(deflatedCenteredX);
            }

            const typename ForwardGspcaSolver::Param forwardParam{
                k, forwardSolver, zero};
            const ForwardGspcaSolver forwardGspca{forwardParam};

            const typename BackwardGspcaSolver::Param backwardParam{
                k, backwardSolver, zero};
            const BackwardGspcaSolver backwardGspca{backwardParam};

            auto forwardSolutionFuture = std::async(
                std::launch::async,
                static_cast<Component (ForwardGspcaSolver::*)(
                    const Matrix &, const ComplementaryProjection &) const>(
                    &ForwardGspcaSolver::run),
                &forwardGspca, centeredX, B);

            Component backwardSolutionSparseEigenElement =
                backwardGspca.run(centeredX, B);

            Component forwardSolutionSparseEigenElement =
                forwardSolutionFuture.get();

            if (forwardSolutionSparseEigenElement.value >=
                backwardSolutionSparseEigenElement.value)
            {
                return forwardSolutionSparseEigenElement;
            }
            else
            {
                return backwardSolutionSparseEigenElement;
            }
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        ParallelGspcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &centeredX,
            const ComplementaryProjection<Scalar> &B, const Param &param,
            ProgressBar *progressBar)
        {
            using Matrix = Matrix<Scalar>;
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;
            using ComplementaryProjection = ComplementaryProjection<Scalar>;

            const Matrix deflatedCenteredX = centeredX * B;

            const auto k = param.k;
            const auto &forwardSolver = param.eigenSolverForForward;
            const auto &backwardSolver = param.eigenSolverForBackward;
            const auto zero = param.zero;
            const auto n = deflatedCenteredX.cols();

            if (k > static_cast<Index>(0))
            {
                ComponentsContainer components;
                components.reserve(n);

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
                    ParallelGspcaSolver<Scalar, EigenSolver, ProgressBar>{param}
                        .run(centeredX, B));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

            const typename ForwardGspcaSolver::Param forwardParam{
                k, forwardSolver, zero};
            const typename BackwardGspcaSolver::Param backwardParam{
                k, backwardSolver, zero};

            auto forwardSolutionFuture = std::async(
                std::launch::async,
                static_cast<ComponentsContainer (*)(
                    const Matrix &, const ComplementaryProjection &,
                    const typename ForwardGspcaSolver::Param &, ProgressBar *)>(
                    &ForwardGspcaSolver::run),
                centeredX, B, forwardParam, nullptr);

            ComponentsContainer backwardSolutionSparseEigenElement =
                BackwardGspcaSolver::run(centeredX, B, backwardParam,
                                         progressBar);

            ComponentsContainer forwardSolutionSparseEigenElement =
                forwardSolutionFuture.get();

            for (Index j = 1; j <= n; ++j)
            {
                if (backwardSolutionSparseEigenElement.at(j).value <
                    forwardSolutionSparseEigenElement.at(j).value)
                {
                    backwardSolutionSparseEigenElement.at(j) =
                        std::move(forwardSolutionSparseEigenElement.at(j));
                }
            }

            return backwardSolutionSparseEigenElement;
        }

        template <std::floating_point ScalarType>
        using BackwardGspca = SparsePC<
            BackwardGspcaSolver<ScalarType, SpectraLibEigenSolver<ScalarType>>>;

        template <std::floating_point ScalarType>
        using ForwardGspca = SparsePC<
            ForwardGspcaSolver<ScalarType, SpectraLibEigenSolver<ScalarType>>>;

        template <std::floating_point ScalarType>
        using ParallelGspca = SparsePC<
            ParallelGspcaSolver<ScalarType, SpectraLibEigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace Sparsepc
