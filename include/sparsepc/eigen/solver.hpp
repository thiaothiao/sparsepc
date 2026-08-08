#pragma once

#include <iostream>
#include <set>
#include <vector>
#include <utility>

#include <Eigen/Dense>

#include <sparsepc/utils/matrix.hpp>

namespace Sparsepc
{
    /**
     * @brief A virtual state for each component.
     * @details It is used essentially during sparse components selection.
     */
    enum class ComponentState : std::uint8_t
    {
        Validated = 0U, ///< selected component for a given round.
        Unvalidated,    ///< nonselected candidates for a given round.
        Unknown         ///< not determined.
    };

    /**
     * @brief A struct that represents a typical solution of a sparse principal
     * component solver.
     *
     * @details It contains the "eigen element" and its state.
     *
     * @tparam ScalarType The used scalar type.
     */
    template <std::floating_point ScalarType> struct Component
    {
        using Scalar = ScalarType;

        Component()
            : state{ComponentState::Unknown}, value{static_cast<Scalar>(0)},
              vector{}, q{}
        {
        }

        Component(const Index n)
            : state{ComponentState::Unknown}, value{static_cast<Scalar>(0)},
              vector{Vector<Scalar>::Zero(n)}, q{}
        {
        }

        Component(Scalar valueInput, const Vector<Scalar> &vectorInput)
            : state{ComponentState::Unknown}, value{valueInput},
              vector{vectorInput}, q{}
        {
        }

        Component(const Component &) = default;
        Component &operator=(const Component &) = default;

        Component(Component &&) = default;
        Component &operator=(Component &&) = default;

        ComponentState state;  ///< the component state.
        Scalar value;          ///< the component explained variance.
        Vector<Scalar> vector; ///< the component "eigen" vector.
        Vector<Scalar> q;      ///< a deflated representation of the component.
    };

    template <class ComponentType>
    using ComponentsContainer = std::unordered_map<Index, ComponentType>;

    template <std::floating_point ScalarType>
    static void printComponents(
        const ComponentsContainer<Component<ScalarType>> &eigenElements)
    {
        for (const auto &[k, component] : eigenElements)
        {
            std::cout << k << "\t" << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    template <class ImplementationType>
    concept EigenSolverLike = requires(ImplementationType impl) {
        {
            std::as_const(impl).maximumValueElement(
                Matrix<typename ImplementationType::Scalar>{})
        }
        -> std::convertible_to<Component<typename ImplementationType::Scalar>>;

        {
            std::as_const(impl).maximumValue(
                Matrix<typename ImplementationType::Scalar>{})
        } -> std::convertible_to<typename ImplementationType::Scalar>;
    };

    /**
     * @brief A class that uses custom Power and Gram methods for eigen elements
     * computations.
     */
    template <std::floating_point ScalarType> class EigenSolver final
    {
      public:
        using Scalar = ScalarType;

        struct Param final
        {
            Param(Scalar epsilonInput = static_cast<Scalar>(1e-4),
                  unsigned int maximumNumberOfIterationsInput = 1000000U)
                : epsilon{epsilonInput},
                  maximumNumberOfIterations{maximumNumberOfIterationsInput}
            {
            }

            Param(const Param &) = default;
            Param &operator=(const Param &) = default;

            Param(Param &&) = default;
            Param &operator=(Param &&) = default;

            const Scalar epsilon;
            const unsigned int maximumNumberOfIterations;
        };

        EigenSolver(const Param &param = {}) : m_Param{param} {}

        auto maximumValue(Matrix<Scalar> sigma) const;

        auto maximumValueElement(const Matrix<Scalar> &sigma) const;

      private:
        Param m_Param;
    };

    template <std::floating_point ScalarType>
    auto EigenSolver<ScalarType>::maximumValue(Matrix<Scalar> sigma) const
    { // Gram iteration
        if (sigma.cols() == static_cast<Index>(1))
        {
            return sigma.value();
        }

        Matrix<Scalar> G = std::move(sigma);

        Scalar r = static_cast<Scalar>(0);

        auto twoPowerMinusCount = static_cast<Scalar>(1);

        unsigned int count = 0;
        Scalar maximumEigenValue = static_cast<Scalar>(-1);
        while (true)
        {
            const auto gNormF = G.norm();

            G /= gNormF;

            G = G.transpose() * G;

            r = static_cast<Scalar>(2) * (r + std::log(gNormF));

            ++count;

            twoPowerMinusCount *= static_cast<Scalar>(0.5);

            const auto eigval = std::pow(G.norm(), twoPowerMinusCount) *
                                std::exp(twoPowerMinusCount * r);

            if (std::abs(maximumEigenValue - eigval) <= m_Param.epsilon)
            {
                break;
            }

            maximumEigenValue = eigval;

            if (count >= m_Param.maximumNumberOfIterations)
            {
                break;
            }
        }

        return maximumEigenValue;
    }

    template <std::floating_point ScalarType>
    auto EigenSolver<ScalarType>::maximumValueElement(
        const Matrix<Scalar> &sigma) const
    { // Power iteration
        const auto n = sigma.cols();

        Component<Scalar> eigenElement(n);
        auto &u = eigenElement.vector;
        auto &value = eigenElement.value;

        u.setOnes();

        value = (u.transpose() * sigma * u).value();

        if (n == static_cast<Index>(1))
        {
            return eigenElement;
        }

        unsigned int count = 0U;
        while (true)
        { // TODO optimize products
            u = sigma * u;
            u.normalize();

            const auto newValue = (u.transpose() * sigma * u).value();

            if (std::abs(value - newValue) <= m_Param.epsilon)
            {
                value = newValue;
                break;
            }

            value = newValue;

            ++count;
            if (count >= m_Param.maximumNumberOfIterations)
            {
                break;
            }
        }

        // preferring non negative max values. <<rectify>> u s signs
        Index idx = static_cast<Index>(-1);
        u.cwiseAbs().maxCoeff(&idx);

        if (u[idx] < static_cast<Scalar>(0))
        {
            u *= static_cast<Scalar>(-1);
        }

        return eigenElement;
    }

    /**
     * @brief A class that uses the C++ library Eigen for eigen elements
     * computations.
     */
    template <std::floating_point ScalarType> class EigenLibEigenSolver final
    {
      public:
        using Scalar = ScalarType;

        EigenLibEigenSolver() {}

        auto maximumValue(const Matrix<Scalar> &sigma) const;

        auto maximumValueElement(const Matrix<Scalar> &sigma) const;
    };

    template <std::floating_point ScalarType>
    inline auto EigenLibEigenSolver<ScalarType>::maximumValue(
        const Matrix<Scalar> &sigma) const
    {
        return (sigma.cols() == static_cast<Index>(1))
                   ? sigma.value()
                   : Eigen::SelfAdjointEigenSolver<Matrix<Scalar>>(sigma)
                         .eigenvalues()[sigma.cols() - 1];
    }

    template <std::floating_point ScalarType>
    auto EigenLibEigenSolver<ScalarType>::maximumValueElement(
        const Matrix<Scalar> &sigma) const
    {
        const auto n = sigma.cols();

        Component<Scalar> eigenElement(n);

        if (n == static_cast<Index>(1))
        {
            eigenElement.value = sigma.value();
            eigenElement.vector.setOnes();
            return eigenElement;
        }

        Eigen::SelfAdjointEigenSolver<Matrix<Scalar>> selfAdjointEigenSolver(
            sigma);
        eigenElement.value = selfAdjointEigenSolver.eigenvalues()[n - 1];
        eigenElement.vector = selfAdjointEigenSolver.eigenvectors().col(
            n - 1); // Why not use move!!!

        // preferring non negative max values. <<rectify>> u s signs
        auto &u = eigenElement.vector;
        Index idx = static_cast<Index>(-1);
        u.cwiseAbs().maxCoeff(&idx);

        if (u[idx] < static_cast<Scalar>(0))
        {
            u *= static_cast<Scalar>(-1);
        }

        return eigenElement;
    }

    template <std::floating_point ScalarType>
    auto sort(const Vector<ScalarType> &v)
    {
        using Scalar = ScalarType;

        auto comparePairsLambda = [](const std::pair<Index, Scalar> &lhs,
                                     const std::pair<Index, Scalar> &rhs) {
            return lhs.second < rhs.second;
        };

        // Declare std::set using decltype for the comparator type
        std::multiset<std::pair<Index, Scalar>, decltype(comparePairsLambda)>
            ss(comparePairsLambda);
        const Index n = v.size();
        for (Index i = 0; i < n; ++i)
        {
            ss.insert(std::pair{i, v[i]});
        }

        std::vector<Index> indices;
        indices.reserve(v.size());

        for (const auto &s : ss)
        {
            indices.push_back(s.first);
        }

        return indices;
    }
} // namespace Sparsepc
