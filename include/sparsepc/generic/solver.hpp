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
        template <class ModelImplementationType>
        concept SparsePCModelLike = requires(ModelImplementationType impl) {
            {
                std::as_const(impl).run(
                    Matrix<typename ModelImplementationType::Scalar>{})
            } -> std::convertible_to<
                Component<typename ModelImplementationType::Scalar>>;

            {
                ModelImplementationType::runAll(
                    Matrix<typename ModelImplementationType::Scalar>{},
                    typename ModelImplementationType::Param{},
                    static_cast<typename ModelImplementationType::ProgressBar
                                    *>(nullptr))
            } -> std::convertible_to<ComponentsContainer<
                Component<typename ModelImplementationType::Scalar>>>;
        }; // TODO bencmarkings on map, unordered_map, flat_map

        template <SparsePCModelLike ModelImplementationType>
        class SparsePC final
        {
          public:
            using ModelImplementation = ModelImplementationType;
            using Scalar = typename ModelImplementation::Scalar;
            using ModelParam = typename ModelImplementation::Param;
            using ProgressBar = typename ModelImplementationType::ProgressBar;

            struct Param final
            {
                Param(std::vector<ModelParam> modelParamsInput)
                    : nbComponents{static_cast<Index>(modelParamsInput.size())},
                      modelParams{modelParamsInput}
                {
                }

                Param(const Param &) = default;
                Param &operator=(const Param &) = default;

                Param(Param &&) = default;
                Param &operator=(Param &&) = default;

                const Index nbComponents;
                const std::vector<ModelParam> modelParams;
            };

            SparsePC(const Param &param) : m_Param{param} {}

            auto run(const Matrix<Scalar> &sigma) const;

            template <class ComponentType>
            static auto computeNextComponentCandidates(
                const Matrix<Scalar> &sigma, const ModelParam &param,
                const std::vector<ComponentType> &validatedComponents,
                ProgressBar *progressBar);

          private:
            const Param m_Param;

            static auto computeComponentCandidates(
                const Matrix<Scalar> &sigma, const ModelParam &param,
                const Matrix<Scalar> &deflatedSigma, const Matrix<Scalar> &B,
                ProgressBar *progressBar);
        };

        template <SparsePCModelLike ModelImplementationType>
        auto SparsePC<ModelImplementationType>::run(
            const Matrix<Scalar> &sigma) const
        {
            using Matrix = Matrix<Scalar>;
            using Component = Component<Scalar>;

            const auto n = static_cast<Index>(sigma.cols());

            std::vector<Component> sparseSolutions;
            sparseSolutions.reserve(m_Param.nbComponents);

            sparseSolutions.push_back(
                ModelImplementationType{m_Param.modelParams[0]}.run(sigma));

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
                    ModelImplementationType{m_Param.modelParams[j]}.run(
                        inverse * B * sigma * B));

                q = B * sparseSolutions.back().vector;

                sparseSolutions.back().value =
                    (q.transpose() * sigma * q).value() / q.squaredNorm();
            }

            return sparseSolutions;
        }

        template <SparsePCModelLike ModelImplementationType>
        auto SparsePC<ModelImplementationType>::computeComponentCandidates(
            const Matrix<Scalar> &sigma, const ModelParam &param,
            const Matrix<Scalar> &deflatedSigma, const Matrix<Scalar> &B,
            ProgressBar *progressBar)
        {
            if (B.size() == static_cast<Scalar>(0))
            {
                auto candidates =
                    ModelImplementationType::runAll(sigma, param, progressBar);

                for (auto &[i, cpnt] : candidates)
                {
                    cpnt.q = cpnt.vector;
                }

                return candidates;
            }
            else
            {
                auto candidates = ModelImplementationType::runAll(
                    deflatedSigma, param, progressBar);

                for (auto &[i, cpnt] : candidates)
                {
                    cpnt.q = B * cpnt.vector;

                    cpnt.value = (cpnt.q.transpose() * sigma * cpnt.q).value() /
                                 cpnt.q.squaredNorm();
                }

                return candidates;
            }
        }

        template <SparsePCModelLike ModelImplementationType>
        template <class ComponentType>
        auto SparsePC<ModelImplementationType>::computeNextComponentCandidates(
            const Matrix<Scalar> &sigma, const ModelParam &param,
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
