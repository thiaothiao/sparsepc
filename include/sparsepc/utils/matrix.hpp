#pragma once

#include <set>
#include <vector>

#include <Eigen/Dense>

namespace Sparsepc
{
    using Index = Eigen::Index;
    using Vectori = Eigen::Matrix<Index, Eigen::Dynamic, 1>;
    template <std::floating_point ScalarType>
    using Vector = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>;
    template <std::floating_point ScalarType>
    using Matrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic,
                                 Eigen::ColMajor>;
    template <std::floating_point ScalarType>
    using RMMatrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic,
                                   Eigen::RowMajor>;

    // represent I- \sum q_sq_s^T
    template <std::floating_point ScalarType> struct ComplementaryProjection
    {
        using Scalar = ScalarType;

        ComplementaryProjection() {}

        ComplementaryProjection(const ComplementaryProjection &) = default;
        ComplementaryProjection &
        operator=(const ComplementaryProjection &) = default;

        ComplementaryProjection(ComplementaryProjection &&) = default;
        ComplementaryProjection &
        operator=(ComplementaryProjection &&) = default;

        auto isIdentity() const { return qs.empty(); }

        ComplementaryProjection &add(const Vector<Scalar> &q)
        {
            qs.push_back(q);
            return *this;
        }

        friend ComplementaryProjection add(ComplementaryProjection lhs,
                                           const Vector<Scalar> &q)
        {
            lhs.add(q);
            return lhs;
        }

        template <class RightHandSideType>
        friend auto operator*(ComplementaryProjection lhs,
                              const RightHandSideType &rhs)
        {
            auto result = rhs;
            for (const auto &q : lhs.qs)
            { // TODO use std accumulation algorithms
              // use openmp parallel
                result -= q * (q.transpose() * rhs);
            }

            return result;
        }

        template <class LeftHandSideType>
        friend auto operator*(const LeftHandSideType &lhs,
                              ComplementaryProjection rhs)
        {
            auto result = lhs;
            for (const auto &q : rhs.qs)
            { // TODO use std accumulation algorithms
              // use openmp parallel
                result -= (lhs * q) * q.transpose();
            }

            return result;
        }

        std::vector<Vector<Scalar>> qs;
    };

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
