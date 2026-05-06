#pragma once

#include <concepts>
#include <utility>
#include <vector>

#include <Sparsepc/eigen/solver.hpp>
#include <Sparsepc/utils/matrix.hpp>

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
         * @param run a static member function that takes a Matrix, a Param, a
         * progress and return a bunch of components.
         * @tparam ImplementationType The type to check.
         */
        template <class ImplementationType>
        concept SparsePCSolverLike =
            requires(ImplementationType impl) {
                {
                    std::as_const(impl).run(
                        Matrix<typename ImplementationType::Scalar>{})
                } -> std::convertible_to<
                    Component<typename ImplementationType::Scalar>>;

                {
                    ImplementationType::run(
                        Matrix<typename ImplementationType::Scalar>{},
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
            using ProgressBar = typename ImplementationType::ProgressBar;

            /**
             * @brief Generic solver parameter set.
             * @details Store parameters needed for each principal component
             * round.
             */
            struct Param final
            {
                Param(std::vector<ImplementationParam> implementationParamsInput)
                    : nbComponents{static_cast<Index>(implementationParamsInput.size())},
                      implementationParams{implementationParamsInput}
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
             * @brief Computes the principal component associated with the
             * parameters.
             * @param sigma The covariance matrix.
             * @return The computed principal component.
             */
            auto run(const Matrix<Scalar> &sigma) const;

            /**
             * @brief Computes a set of candidates for the next round principal
             * component.
             * @param sigma The covariance matrix.
             * @param param The parameters to be used during the computations.
             * @param previousRoundComponents The previous rounds principal
             * components.
             * @param progressBar The computation progress reporter.
             * @return The computed next round candidates.
             */
            template <class ComponentType>
            static auto computeNextComponentCandidates(
                const Matrix<Scalar> &sigma, const ImplementationParam &param,
                const std::vector<ComponentType> &previousRoundComponents,
                ProgressBar *progressBar);

          private:
            const Param m_Param;

            /**
             * @brief Computes a set of candidates. Does not know about rounds.
             * @param sigma The covariance matrix.
             * @param param The parameters to be used during the computations.
             * @param deflatedSigma The the delated covariance matrix.
             * @param B The matrix that accumulates (I - projections).
             * @param progressBar The computation progress reporter.
             * @return The computed next round candidates.
             */
            static auto computeComponentCandidates(
                const Matrix<Scalar> &sigma, const ImplementationParam &param,
                const Matrix<Scalar> &deflatedSigma, const Matrix<Scalar> &B,
                ProgressBar *progressBar);
        };

        template <SparsePCSolverLike ImplementationType>
        auto
        SparsePC<ImplementationType>::run(const Matrix<Scalar> &sigma) const
        {
            using Matrix = Matrix<Scalar>;
            using Component = Component<Scalar>;

            const auto n = static_cast<Index>(sigma.cols());

            std::vector<Component> sparseSolutions;
            sparseSolutions.reserve(m_Param.nbComponents);

            sparseSolutions.push_back(
                ImplementationType{m_Param.implementationParams[0]}.run(sigma));

            Matrix B = Matrix::Identity(n, n);
            Matrix inverse = Matrix::Zero(n, n); // TODO optimize this block
            inverse.diagonal().array() =
                static_cast<Scalar>(1) /
                (static_cast<Scalar>(1) + static_cast<Scalar>(1e-4));

            auto q = sparseSolutions.back().vector;

            for (Index j = 1; j < m_Param.nbComponents; ++j)
            {
                B -= q * q.transpose();

                const auto inverseQ = inverse * q;

                // Woodbury matrix identity
                inverse += ((static_cast<Scalar>(1) /
                             (static_cast<Scalar>(1) - q.dot(inverseQ))) *
                            inverseQ) *
                           inverseQ.transpose();

                sparseSolutions.push_back(
                    ImplementationType{m_Param.implementationParams[j]}.run(inverse * B *
                                                                   sigma * B));

                q = B * sparseSolutions.back().vector;

                sparseSolutions.back().value =
                    (q.transpose() * sigma * q).value() / q.squaredNorm();
            }

            return sparseSolutions;
        }

        template <SparsePCSolverLike ImplementationType>
        auto SparsePC<ImplementationType>::computeComponentCandidates(
            const Matrix<Scalar> &sigma, const ImplementationParam &param,
            const Matrix<Scalar> &deflatedSigma, const Matrix<Scalar> &B,
            ProgressBar *progressBar)
        {
            if (B.size() == static_cast<Scalar>(0))
            {
                auto candidates =
                    ImplementationType::run(sigma, param, progressBar);

                for (auto &[i, cpnt] : candidates)
                {
                    cpnt.q = cpnt.vector;
                }

                return candidates;
            }
            else
            {
                auto candidates =
                    ImplementationType::run(deflatedSigma, param, progressBar);

                for (auto &[i, cpnt] : candidates)
                {
                    cpnt.q = B * cpnt.vector;

                    cpnt.value = (cpnt.q.transpose() * sigma * cpnt.q).value() /
                                 cpnt.q.squaredNorm();
                }

                return candidates;
            }
        }

        template <SparsePCSolverLike ImplementationType>
        template <class ComponentType>
        auto SparsePC<ImplementationType>::computeNextComponentCandidates(
            const Matrix<Scalar> &sigma, const ImplementationParam &param,
            const std::vector<ComponentType> &validatedComponents,
            ProgressBar *progressBar)
        {
            using Matrix = Matrix<Scalar>;
            using Component = Component<Scalar>;

            const auto n = sigma.cols();

            if (validatedComponents.empty())
            {
                return SparsePC::computeComponentCandidates(sigma, param, sigma,
                                                            {}, progressBar);
            }

            Matrix B = Matrix::Identity(n, n);
            Matrix inverse = Matrix::Zero(n, n); // TODO optimize this block
            inverse.diagonal().array() =
                static_cast<Scalar>(1) /
                (static_cast<Scalar>(1) + static_cast<Scalar>(1e-4));

            for (const Component &validatedComponent : validatedComponents)
            {
                const auto &q = validatedComponent.q;

                B -= q * q.transpose();

                const auto inverseQ = inverse * q;

                // Woodbury matrix identity
                inverse += ((static_cast<Scalar>(1) /
                             (static_cast<Scalar>(1) - q.dot(inverseQ))) *
                            inverseQ) *
                           inverseQ.transpose();
            }

            return SparsePC::computeComponentCandidates(
                sigma, param, inverse * B * sigma * B, B, progressBar);
        }
    } // namespace linearmodel
} // namespace Sparsepc
