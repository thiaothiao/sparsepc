#pragma once

#include <iostream>
#include <utility>

#include <Eigen/Dense>
#include <Eigen/SVD>
#include <Spectra/contrib/PartialSVDSolver.h>

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
    };

    /**
     * @brief A class that uses the C++ library Spectra for eigen elements
     * computations.
     */
    template <std::floating_point ScalarType> class SpectraLibEigenSolver final
    {
      public:
        using Scalar = ScalarType;

        struct Param final
        {
            Param(Index ncvInput = static_cast<Index>(2)) : ncv{ncvInput} {}

            Param(const Param &) = default;
            Param &operator=(const Param &) = default;

            Param(Param &&) = default;
            Param &operator=(Param &&) = default;

            const Index ncv;
        };

        SpectraLibEigenSolver(const Param &param = {}) : m_Param{param} {}

        auto maximumValueElement(const Matrix<Scalar> &featureMatrix) const;

      private:
        const Param m_Param;
    };

    template <std::floating_point ScalarType>
    auto SpectraLibEigenSolver<ScalarType>::maximumValueElement(
        const Matrix<Scalar> &featureMatrix) const
    {
        using Matrix = Matrix<Scalar>;
        const auto n = featureMatrix.cols();

        Component<Scalar> eigenElement(n);

        if (n == static_cast<Index>(1))
        {
            eigenElement.value = featureMatrix.col(0).squaredNorm();
            eigenElement.vector.setOnes();
            return eigenElement;
        }

        Spectra::PartialSVDSolver<Matrix> svds(featureMatrix, 1, m_Param.ncv);
        if (svds.compute() == 1)
        {
            const auto maxSingularValue = svds.singular_values()[0];
            eigenElement.value = maxSingularValue * maxSingularValue;
            eigenElement.vector = svds.matrix_V(1).col(0);
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

        struct Param final
        {
            Param(bool useJacobiSVDInput = false)
                : useJacobiSVD{useJacobiSVDInput}
            {
            }

            Param(const Param &) = default;
            Param &operator=(const Param &) = default;

            Param(Param &&) = default;
            Param &operator=(Param &&) = default;

            const bool useJacobiSVD;
        };

        EigenLibEigenSolver(const Param &param = {}) : m_Param{param} {}

        auto maximumValueElement(const Matrix<Scalar> &featureMatrix) const;

      private:
        const Param m_Param;
    };

    template <std::floating_point ScalarType>
    auto EigenLibEigenSolver<ScalarType>::maximumValueElement(
        const Matrix<Scalar> &featureMatrix) const
    {
        using Matrix = Matrix<Scalar>;
        using Component = Component<Scalar>;

        const auto n = featureMatrix.cols();
        Component eigenElement(n);

        if (n == static_cast<Index>(1))
        {
            eigenElement.value = featureMatrix.col(0).squaredNorm();
            eigenElement.vector.setOnes();
            return eigenElement;
        }

        if (m_Param.useJacobiSVD)
        {
            Eigen::JacobiSVD<Matrix, Eigen::ComputeThinV> svd(featureMatrix);

            const auto maxSingularValue = svd.singularValues()[0];
            eigenElement.value = maxSingularValue * maxSingularValue;
            eigenElement.vector = svd.matrixV().col(0);
        }
        else
        {
            Eigen::BDCSVD<Matrix, Eigen::ComputeThinV> svd(featureMatrix);

            const auto maxSingularValue = svd.singularValues()[0];
            eigenElement.value = maxSingularValue * maxSingularValue;
            eigenElement.vector = svd.matrixV().col(0);
        }

        return eigenElement;
    }
} // namespace Sparsepc
