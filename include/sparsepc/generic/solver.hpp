#pragma once

#include <concepts>
#include <utility>
#include <vector>

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace Sparsepc
{
    namespace linearmodel
    {
        /**
         * @brief Concept for a type that can be used as sparse principal
         * component solver in the generic class SparsePC.
         * @details This concept ensures the type supports
         * @param Scalar an inner type used as scalar type.
         * @param Param an inner type that concentrates a set of parameters.
         * @param ProgressBar an inner type that reports progress level.
         * @param run a const member function that takes a Matrix and return a
         * component.
         * @param run a static member function that takes a Matrix, a
         * ComplementaryProjection, a Param, a progress and return a bunch of
         * components.
         * @tparam ImplementationType The type to check.
         */
        template <class ImplementationType>
        concept SparsePCSolverLike =
            requires(ImplementationType impl) {
                {
                    std::as_const(impl).run(
                        Matrix<typename ImplementationType::Scalar>{},
                        ComplementaryProjection<
                            typename ImplementationType::Scalar>{})
                } -> std::convertible_to<
                    Component<typename ImplementationType::Scalar>>;

                {
                    ImplementationType::run(
                        Matrix<typename ImplementationType::Scalar>{},
                        ComplementaryProjection<
                            typename ImplementationType::Scalar>{},
                        typename ImplementationType::Param{},
                        static_cast<typename ImplementationType::ProgressBar *>(
                            nullptr))
                } -> std::convertible_to<ComponentsContainer<
                    Component<typename ImplementationType::Scalar>>>;
            }; // TODO bencmarkings on map, unordered_map, flat_map

        /**
         * @brief A generic class for various sparse principal component
         * solvers.
         * @details It implements a static polymorphism on concrete solver
         * implementations.
         *
         * @tparam ImplementationType A sparse principal component solver like
         * implementation.
         */
        template <SparsePCSolverLike ImplementationType>
        class SparsePC final
        {
          public:
            using Implementation = ImplementationType;
            using Scalar = typename Implementation::Scalar;
            using ImplementationParam = typename Implementation::Param;
            using ProgressBar = typename Implementation::ProgressBar;

            /**
             * @brief Generic solver parameter set.
             * @details Store parameters needed for each principal component
             * round.
             */
            struct Param final
            {
                Param(std::vector<ImplementationParam> implementationParamsInput)
                    : nbComponents{static_cast<Index>(implementationParamsInput.size())},
                      implementationParams{std::move(implementationParamsInput)}
                {
                }

                Param(const Param &) = default;
                Param &operator=(const Param &) = default;

                Param(Param &&) = default;
                Param &operator=(Param &&) = default;

                const Index
                    nbComponents; /**< Number of rounds (or components) */
                const std::vector<ImplementationParam>
                    implementationParams; /**< Container for rounds parameters
                                           */
            };

            /**
             * @brief Constructs a new SparsePC object with specified
             * parameters.
             * @param param The parameters used by the solver object.
             */
            SparsePC(const Param &param) : m_Param{param} {}

            /**
             * @brief Computes the principal components associated with the
             * parameters.
             * @details The components are computed sequentially, deflating the
             * covariance matrix with the previously found components.
             * @param featureMatrix The colwise feature matrix.
             * @return The computed principal components.
             */
            auto run(const Matrix<Scalar> &featureMatrix) const;

            /**
             * @brief Computes a set of candidates for the next round principal
             * component.
             * @param featureMatrix The colwise feature matrix.
             * @param param The parameters to be used during the computations.
             * @param validatedComponents The previous rounds validated
             * components.
             * @param progressBar The computation progress reporter.
             * @return The computed next round candidates.
             */
            template <class ComponentType>
            static auto computeNextComponentCandidates(
                const Matrix<Scalar> &featureMatrix,
                const ImplementationParam &param,
                const std::vector<ComponentType> &validatedComponents,
                ProgressBar *progressBar);

          private:
            const Param m_Param;
        };

        template <SparsePCSolverLike ImplementationType>
        auto SparsePC<ImplementationType>::run(
            const Matrix<Scalar> &featureMatrix) const
        {
            using Component = Component<Scalar>;
            using Vector = Vector<Scalar>;

            std::vector<Component> sparseSolutions;
            sparseSolutions.reserve(m_Param.nbComponents);

            auto complementaryProjectionMatrix =
                ComplementaryProjection<Scalar>();

            for (Index j = 0; j < m_Param.nbComponents; ++j)
            {
                sparseSolutions.push_back(
                    ImplementationType{m_Param.implementationParams[j]}.run(
                        featureMatrix, complementaryProjectionMatrix));

                const Vector q = complementaryProjectionMatrix *
                                 sparseSolutions.back().vector;

                sparseSolutions.back().value =
                    (featureMatrix * q).squaredNorm() / q.squaredNorm();

                complementaryProjectionMatrix.add(q);
            }

            return sparseSolutions;
        }

        template <SparsePCSolverLike ImplementationType>
        template <class ComponentType>
        auto SparsePC<ImplementationType>::computeNextComponentCandidates(
            const Matrix<Scalar> &featureMatrix,
            const ImplementationParam &param,
            const std::vector<ComponentType> &validatedComponents,
            ProgressBar *progressBar)
        {
            using ComplementaryProjection = ComplementaryProjection<Scalar>;
            using Component = Component<Scalar>;

            auto complementaryProjectionMatrix = ComplementaryProjection();

            for (const Component &validatedComponent : validatedComponents)
            {
                const auto &q = validatedComponent.q;

                complementaryProjectionMatrix.add(q);
            }

            auto candidates = ImplementationType::run(
                featureMatrix, complementaryProjectionMatrix, param,
                progressBar);

            // update explained variances
            for (auto &[i, cpnt] : candidates)
            {
                cpnt.q = complementaryProjectionMatrix * cpnt.vector;

                cpnt.value = (featureMatrix * cpnt.q).squaredNorm() /
                             cpnt.q.squaredNorm();
            }

            return candidates;
        }
    } // namespace linearmodel
} // namespace Sparsepc
