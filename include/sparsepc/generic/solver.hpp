#pragma once

#include <concepts>
#include <utility>
#include <vector>

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace sparsepc
{
    namespace linearmodel
    {
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
        template <SparsePCSolverLike ImplementationType>
        class SparsePC final
        {
          public:
            using Implementation = ImplementationType;
            using Scalar = typename Implementation::Scalar;
            using ImplementationParam = typename Implementation::Param;
            using ProgressBar = typename ImplementationType::ProgressBar;

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

            SparsePC(const Param &param) : m_Param{param} {}

            auto run(const Matrix<Scalar> &sigma) const;

            template <class ComponentType>
            static auto computeNextComponentCandidates(
                const Matrix<Scalar> &sigma, const ImplementationParam &param,
                const std::vector<ComponentType> &previousRoundComponents,
                ProgressBar *progressBar);

          private:
            const Param m_Param;

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
} // namespace sparsepc
