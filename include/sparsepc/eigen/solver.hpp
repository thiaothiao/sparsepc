#pragma once

#include <iostream>
#include <set>
#include <vector>
#include <utility>

#include <Eigen/Dense>
#include <Eigen/SVD>

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

        auto maximumValue(const Matrix<Scalar> &centeredFeatureMatrix) const;

        auto
        maximumValueElement(const Matrix<Scalar> &centeredFeatureMatrix) const;

      private:
        Param m_Param;
    };

    template <std::floating_point ScalarType>
    auto EigenSolver<ScalarType>::maximumValue(
        const Matrix<Scalar> &centeredFeatureMatrix) const
    { // Gram iteration
        using Matrix = Matrix<Scalar>;
        using Vector = Vector<Scalar>;
        using Component = Component<Scalar>;

        Matrix G = centeredFeatureMatrix.transpose() *
                   centeredFeatureMatrix;

        if (G.cols() == static_cast<Index>(1))
        {
            return G.value();
        }

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
        const Matrix<Scalar> &centeredFeatureMatrix) const
    { // Power iteration
        using Matrix = Matrix<Scalar>;
        using Vector = Vector<Scalar>;
        using Component = Component<Scalar>;

        const auto n = centeredFeatureMatrix.cols();

        Component eigenElement(n);
        auto &u = eigenElement.vector;
        auto &value = eigenElement.value;

        u.setOnes();

        value = (centeredFeatureMatrix * u).squaredNorm();

        if (n == static_cast<Index>(1))
        {
            return eigenElement;
        }

        unsigned int count = 0U;
        while (true)
        { // TODO optimize products
            u = centeredFeatureMatrix.transpose() * (centeredFeatureMatrix * u);
            u.normalize();

            const auto newValue = (centeredFeatureMatrix * u).squaredNorm();

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

        auto maximumValue(const Matrix<Scalar> &centeredFeatureMatrix) const;

        auto
        maximumValueElement(const Matrix<Scalar> &centeredFeatureMatrix) const;
    };

    template <std::floating_point ScalarType>
    inline auto EigenLibEigenSolver<ScalarType>::maximumValue(
        const Matrix<Scalar> &centeredFeatureMatrix) const
    {
        return (centeredFeatureMatrix.cols() == static_cast<Index>(1))
                   ? centeredFeatureMatrix.col(0).squaredNorm()
                   : std::pow(
                         Eigen::JacobiSVD<Matrix<Scalar>>(centeredFeatureMatrix)
                             .singularValues()(0),
                         2);
    }

    template <std::floating_point ScalarType>
    auto EigenLibEigenSolver<ScalarType>::maximumValueElement(
        const Matrix<Scalar> &centeredFeatureMatrix) const
    {
        using Matrix = Matrix<Scalar>;
        using Vector = Vector<Scalar>;
        using Component = Component<Scalar>;

        const auto n = centeredFeatureMatrix.cols();
        Component eigenElement(n);

        if (n == static_cast<Index>(1))
        {
            eigenElement.value = centeredFeatureMatrix.col(0).squaredNorm();
            eigenElement.vector.setOnes();
            return eigenElement;
        }

        Eigen::JacobiSVD<Matrix> svd(centeredFeatureMatrix,
                                     Eigen::ComputeThinV);

        const auto maxSingularValue = svd.singularValues()(0);
        eigenElement.value = maxSingularValue * maxSingularValue;
        eigenElement.vector = svd.matrixV().col(0);

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
