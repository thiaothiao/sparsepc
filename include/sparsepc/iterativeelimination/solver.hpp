#pragma once

#include <concepts>
#include <cstdlib>

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/generic/solver.hpp>
#include <sparsepc/progress/bar.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace Sparsepc
{
    namespace linearmodel
    {
        /**
         * @brief Concept for a type that can be used as an elimination
         * criteria.
         * @details This concept ensures the type supports
         * @param Scalar an inner type used as scalar type.
         * @param index a const member function that takes a Matrix, a Component
         * and return an Index.
         * @tparam ImplementationType The type to check.
         */
        template <class ImplementationType>
        concept EliminationCriterionLike = requires(ImplementationType impl) {
            {
                std::as_const(impl).index(
                    Matrix<typename ImplementationType::Scalar>{},
                    Component<typename ImplementationType::Scalar>{})
            } -> std::convertible_to<Index>;
        };

        /**
         * @brief Iterative Elimination Solver class.
         * @details see Wang et al. 2012 for technical details.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  EliminationCriterionLike EliminationCriterionType,
                  ProgressBarLike ProgressBarType>
        class IterativeEliminationSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using EliminationCriterion = EliminationCriterionType;
            using ProgressBar = ProgressBarType;

            /**
             * @brief IterativeEliminationSolver parameter set.
             * @details Store the parameters needed for the computations.
             */
            struct Param final
            {
                Param(
                    Index kInput = static_cast<Index>(-1),
                    const EigenSolver &eigenSolverInput = {},
                    const EliminationCriterion &eliminationCriterionInput = {},
                    Scalar zeroInput = static_cast<Scalar>(1e-6))
                    : k{kInput}, eigenSolver{eigenSolverInput},
                      eliminationCriterion{eliminationCriterionInput},
                      zero{zeroInput}
                {
                }

                Param(const Param &) = default;
                Param &operator=(const Param &) = default;

                Param(Param &&) = default;
                Param &operator=(Param &&) = default;

                const Index k;
                const EigenSolver eigenSolver;
                const EliminationCriterion eliminationCriterion;
                const Scalar zero;
            };

            /**
             * @brief Constructs a new IterativeEliminationSolver object with
             * specified parameters.
             * @param param The parameters used by the solver object.
             */
            IterativeEliminationSolver(const Param &param = {}) : m_Param{param}
            {
            }

            /**
             * @brief Computes the principal component associated with the
             * parameters.
             * @param featureMatrix The colwise feature matrix.
             * @param complementaryProjectionMatrix The complementary
             * projection.
             * @return The computed principal component.
             */
            auto run(const Matrix<Scalar> &featureMatrix,
                     const ComplementaryProjection<Scalar>
                         &complementaryProjectionMatrix) const;

            /**
             * @brief Computes a set of principal component candidates.
             * @param featureMatrix The colwise feature matrix.
             * @param complementaryProjectionMatrix The complementary
             * projection.
             * @param param The parameters to be used.
             * @param progressBar The computation progress reporter.
             * @return The computed candidates.
             */
            static auto run(const Matrix<Scalar> &featureMatrix,
                            const ComplementaryProjection<Scalar>
                                &complementaryProjectionMatrix,
                            const Param &param, ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  EliminationCriterionLike EliminationCriterionType,
                  ProgressBarLike ProgressBarType>
        auto IterativeEliminationSolver<
            ScalarType, EigenSolverType, EliminationCriterionType,
            ProgressBarType>::run(const Matrix<Scalar> &featureMatrix,
                                  const ComplementaryProjection<Scalar>
                                      &complementaryProjectionMatrix) const
        {
            using Component = Component<Scalar>;
            using Matrix = Matrix<Scalar>;

            const Matrix deflatedFeatureMatrix =
                featureMatrix * complementaryProjectionMatrix;

            const auto k = m_Param.k;
            const auto &eigenSolver = m_Param.eigenSolver;
            const auto &eliminationCriterion = m_Param.eliminationCriterion;

            const auto n = deflatedFeatureMatrix.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(deflatedFeatureMatrix);
            }

            Vectori choosenIndices = Vectori::LinSpaced(n, 0, n - 1);
            auto j0 = static_cast<Index>(0);
            while (true)
            {
                const auto candidateIndices = choosenIndices.tail(
                    static_cast<Index>(choosenIndices.size()) - j0);
                const auto &subSigma = deflatedFeatureMatrix(
                    Eigen::placeholders::all, candidateIndices);
                const auto subElement =
                    eigenSolver.maximumValueElement(subSigma);

                const auto index = candidateIndices[eliminationCriterion.index(
                    subSigma, subElement)];
                if (index != choosenIndices[j0])
                {
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
                deflatedFeatureMatrix(Eigen::placeholders::all, kFoundIndices));

            Component component(n);
            component.value = subDimEigenElement.value;
            component.vector(kFoundIndices) = subDimEigenElement.vector;

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  EliminationCriterionLike EliminationCriterionType,
                  ProgressBarLike ProgressBarType>
        auto IterativeEliminationSolver<
            ScalarType, EigenSolverType, EliminationCriterionType,
            ProgressBarType>::run(const Matrix<Scalar> &featureMatrix,
                                  const ComplementaryProjection<Scalar>
                                      &complementaryProjectionMatrix,
                                  const Param &param, ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;

            const auto deflatedFeatureMatrix =
                featureMatrix * complementaryProjectionMatrix;

            const auto k = param.k;
            const auto &eigenSolver = param.eigenSolver;
            const auto &eliminationCriterion = param.eliminationCriterion;

            const auto n = deflatedFeatureMatrix.cols();

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
                    IterativeEliminationSolver<
                        Scalar, EigenSolver, EliminationCriterion, ProgressBar>{
                        param}
                        .run(featureMatrix, complementaryProjectionMatrix));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

            components.try_emplace(
                n, eigenSolver.maximumValueElement(deflatedFeatureMatrix));

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
                const auto candidateIndices = choosenIndices.tail(
                    static_cast<Index>(choosenIndices.size()) - j0);
                const auto &subSigma = deflatedFeatureMatrix(
                    Eigen::placeholders::all, candidateIndices);
                const auto subElement =
                    eigenSolver.maximumValueElement(subSigma);

                const auto index = candidateIndices[eliminationCriterion.index(
                    subSigma, subElement)];
                if (index != choosenIndices[j0])
                {
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

                auto subDimEigenElement =
                    eigenSolver.maximumValueElement(deflatedFeatureMatrix(
                        Eigen::placeholders::all, kFoundIndices));

                auto &component = components.try_emplace(kCurrent, Component(n))
                                      .first->second;
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

        /**
         * @brief A class that defines the minimal absolute value (MAV)
         * criterion.
         */
        template <std::floating_point ScalarType> class MAVCriterion final
        {
          public:
            using Scalar = ScalarType;
            MAVCriterion() = default;
            auto index(const Matrix<Scalar> &featureMatrix,
                       const Component<Scalar> &component) const;
        };

        template <std::floating_point ScalarType>
        auto MAVCriterion<ScalarType>::index(
            [[maybe_unused]] const Matrix<Scalar> &featureMatrix,
            const Component<Scalar> &component) const
        {
            auto indexMin = static_cast<Index>(-1);
            component.vector.cwiseAbs().minCoeff(&indexMin);
            return indexMin;
        }

        /**
         * @brief A class that defines the approximated minimal variance loss
         * (AMVL) criterion.
         */
        template <std::floating_point ScalarType> class AMVLCriterion final
        {
          public:
            using Scalar = ScalarType;
            AMVLCriterion() = default;
            auto index(const Matrix<Scalar> &featureMatrix,
                       const Component<Scalar> &component) const;
        };

        template <std::floating_point ScalarType>
        auto AMVLCriterion<ScalarType>::index(
            const Matrix<Scalar> &featureMatrix,
            const Component<Scalar> &component) const
        {
            auto indexMin = static_cast<Index>(-1);
            const auto &vSquared = component.vector.cwiseAbs2().array();
            (vSquared *
             (featureMatrix.colwise().squaredNorm().transpose().array() -
              component.value) /
             (vSquared - static_cast<Scalar>(1)))
                .matrix()
                .minCoeff(&indexMin);
            return indexMin;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        using MavIterativeEliminationSolver =
            IterativeEliminationSolver<ScalarType, EigenSolverType,
                                       MAVCriterion<ScalarType>,
                                       ProgressBarType>;

        template <std::floating_point ScalarType>
        using MavIterativeElimination = SparsePC<MavIterativeEliminationSolver<
            ScalarType, SpectraLibEigenSolver<ScalarType>>>;

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        using AmvlIterativeEliminationSolver =
            IterativeEliminationSolver<ScalarType, EigenSolverType,
                                       AMVLCriterion<ScalarType>,
                                       ProgressBarType>;

        template <std::floating_point ScalarType>
        using AmvlIterativeElimination =
            SparsePC<AmvlIterativeEliminationSolver<
                ScalarType, SpectraLibEigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace Sparsepc
