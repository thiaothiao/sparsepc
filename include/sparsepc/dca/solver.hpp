#pragma once

#include <algorithm>
#include <concepts>

#include <sparsepc/eigen/solver.hpp>
#include <sparsepc/generic/solver.hpp>
#include <sparsepc/progress/bar.hpp>
#include <sparsepc/utils/matrix.hpp>

namespace Sparsepc
{
    namespace linearmodel
    {
        /**
         * @brief A Dca method class.
         * @details It implements a Dca method applied to the constrained sparse
         * principal component problem.
         * @tparam ScalarType The considered scalar type.
         * @tparam EigenSolverType The eigen solver to be used for eigen
         * elements computation.
         * @tparam ProgressBarType The progress reporter.
         */
        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType = DummyProgressBar>
        class DcaSolver final
        {
          public:
            using Scalar = ScalarType;
            using EigenSolver = EigenSolverType;
            using ProgressBar = ProgressBarType;

            /**
             * @brief DcaSolver parameter set.
             * @details Store parameters needed for the computations.
             */
            struct Param final
            {
                Param(Index kInput = static_cast<Index>(-1),
                      const EigenSolver &eigenSolverInput = {},
                      Scalar tInput = static_cast<Scalar>(1000000),
                      Scalar toleranceInput = static_cast<Scalar>(1e-4),
                      unsigned int maximumNumberOfIterationsInput = 10000U,
                      Scalar zeroInput = static_cast<Scalar>(1e-6))
                    : k{kInput}, eigenSolver{eigenSolverInput}, t{tInput},
                      tolerance{toleranceInput},
                      maximumNumberOfIterations{maximumNumberOfIterationsInput},
                      zero{zeroInput}
                {
                }

                Param(const Param &) = default;
                Param &operator=(const Param &) = default;

                Param(Param &&) = default;
                Param &operator=(Param &&) = default;

                const Index k;
                const EigenSolver eigenSolver;
                const Scalar t;
                const Scalar tolerance;
                const unsigned int maximumNumberOfIterations;
                const Scalar zero;
            };

            /**
             * @brief Constructs a new DcaSolver object with the specified
             * parameters.
             * @param param The parameters used by the solver object.
             */
            DcaSolver(const Param &param = {}) : m_Param{param} {}

            /**
             * @brief Computes the principal component associated with the
             * parameters.
             * @param featureMatrix The colwise feature matrix.
             * @param complementaryProjectionMatrix The complementary
             * projection.
             * @param guess The starting point of Dca if nonempty.
             * @return The computed principal component.
             */
            auto run(const Matrix<Scalar> &featureMatrix,
                     const ComplementaryProjection<Scalar>
                         &complementaryProjectionMatrix,
                     const Component<Scalar> &guess = {}) const;

            /**
             * @brief Computes a set of principal component candidates.
             * @param featureMatrix The featurewise centered matrix.
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

            struct PrimalSolution
            {
                PrimalSolution(const Index n = static_cast<Index>(0))
                    : x{Vector<Scalar>::Zero(n)}, u{Vector<Scalar>::Zero(n)}
                {
                }

                PrimalSolution(const Vector<Scalar> &xInput,
                               const Vector<Scalar> &uInput)
                    : x{xInput}, u{uInput}
                {
                }

                PrimalSolution(Vector<Scalar> &&xInput, Vector<Scalar> &&uInput)
                    : x{std::move(xInput)}, u{std::move(uInput)}
                {
                }

                PrimalSolution(const PrimalSolution &) = default;
                PrimalSolution &operator=(const PrimalSolution &) = default;

                PrimalSolution(PrimalSolution &&) = default;
                PrimalSolution &operator=(PrimalSolution &&) = default;

                Vector<Scalar> x;
                Vector<Scalar> u;
            };

          private:
            struct DualSolution
            {
                DualSolution(const Index n = static_cast<Index>(0))
                    : q{Vector<Scalar>::Zero(n)}, y{Vector<Scalar>::Zero(n)}
                {
                }

                DualSolution(const Vector<Scalar> &qInput,
                             const Vector<Scalar> &yInput)
                    : q{qInput}, y{yInput}
                {
                }

                DualSolution(Vector<Scalar> &&qInput, Vector<Scalar> &&yInput)
                    : q{std::move(qInput)}, y{std::move(yInput)}
                {
                }

                DualSolution(const DualSolution &) = default;
                DualSolution &operator=(const DualSolution &) = default;

                DualSolution(DualSolution &&) = default;
                DualSolution &operator=(DualSolution &&) = default;

                Vector<Scalar> q;
                Vector<Scalar> y;
            };

            auto objectiveValue(const Matrix<Scalar> &featureMatrix,
                                const PrimalSolution &solution, double t) const;
            auto kktCandidate(const Vector<Scalar> &q, const Vector<Scalar> &y,
                              Scalar r) const;
            auto computePhir(const Vector<Scalar> &y, Scalar r,
                             const PrimalSolution &solution) const;
            auto dual(const Matrix<Scalar> &featureMatrix,
                      const PrimalSolution &solution, double t) const;
            auto primal([[maybe_unused]] const Matrix<Scalar> &featureMatrix,
                        const DualSolution &dualSolution) const;

            const Param m_Param;
        };

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        inline auto
        DcaSolver<ScalarType, EigenSolverType, ProgressBarType>::objectiveValue(
            const Matrix<Scalar> &featureMatrix, const PrimalSolution &solution,
            double t) const
        {
            return -(featureMatrix * solution.x).squaredNorm() -
                   t * solution.u.squaredNorm();
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto DcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &featureMatrix,
            const ComplementaryProjection<Scalar>
                &complementaryProjectionMatrix,
            const Component<Scalar> &guess) const
        {
            using Component = Component<Scalar>;
            using Matrix = Matrix<Scalar>;

            const Matrix deflatedFeatureMatrix =
                featureMatrix * complementaryProjectionMatrix;

            const auto k = m_Param.k;
            const auto &eigenSolver = m_Param.eigenSolver;

            const auto n = deflatedFeatureMatrix.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(deflatedFeatureMatrix);
            }

            if (static_cast<Index>(1) == n)
            {
                Component cmponent(n);
                cmponent.value = deflatedFeatureMatrix.col(0).squaredNorm();
                cmponent.vector.setOnes();

                return cmponent;
            }

            Component component =
                (guess.vector.size() == static_cast<Index>(0))
                    ? eigenSolver.maximumValueElement(deflatedFeatureMatrix)
                    : guess;

            PrimalSolution primalSsolution(n);
            primalSsolution.x = std::move(component.vector); // std::move()
            primalSsolution.u.setOnes();

            auto t = static_cast<Scalar>(2) * component.value;

            while (t <= m_Param.t)
            {
                auto previousObj =
                    objectiveValue(deflatedFeatureMatrix, primalSsolution, t);

                unsigned int count = 0U;
                while (true)
                {
                    auto dualSolution =
                        dual(deflatedFeatureMatrix, primalSsolution, t);

                    primalSsolution =
                        primal(deflatedFeatureMatrix, dualSolution);

                    auto obj = objectiveValue(deflatedFeatureMatrix,
                                              primalSsolution, t);

                    if (std::abs(previousObj - obj) <= m_Param.tolerance)
                    {
                        break;
                    }

                    previousObj = obj;

                    ++count;
                    if (count > m_Param.maximumNumberOfIterations)
                    {
                        break;
                    }
                }

                if ((primalSsolution.u.array() > static_cast<Scalar>(1) - 1e-5)
                        .count() <= m_Param.k)
                {
                    break;
                }

                t *= static_cast<Scalar>(10);
            }

            component.vector = std::move(primalSsolution.x);

            component.vector.normalize();

            component.value =
                (deflatedFeatureMatrix * component.vector).squaredNorm();

            return component;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto DcaSolver<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar> &featureMatrix,
            const ComplementaryProjection<Scalar>
                &complementaryProjectionMatrix,
            const Param &param, ProgressBar *progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;
            using Matrix = Matrix<Scalar>;

            const Matrix deflatedFeatureMatrix =
                featureMatrix * complementaryProjectionMatrix;

            const auto &eigenSolver = param.eigenSolver;
            const auto k = param.k;
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
                    DcaSolver<Scalar, EigenSolver, ProgressBar>{param}.run(
                        featureMatrix, complementaryProjectionMatrix));

                if (progressBar)
                {
                    progressBar->setValue(n);

                    progressBar->processEvents();
                }

                return components;
            }

            const auto &component =
                components
                    .try_emplace(n, eigenSolver.maximumValueElement(
                                        deflatedFeatureMatrix))
                    .first->second;

            if (n == 1)
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

            // TODO use #pragma omp parallel and #pragma omp cancel
            for (Index k = 1; k < n; ++k)
            {
                components.try_emplace(
                    k,
                    DcaSolver<Scalar, EigenSolver, ProgressBar>{
                        Param{k, param.eigenSolver, param.t, param.tolerance,
                              param.maximumNumberOfIterations, param.zero}}
                        .run(featureMatrix, complementaryProjectionMatrix,
                             component));

                if (progressBar)
                {
                    progressBar->setValue(static_cast<int>(k) + 1);

                    progressBar->processEvents();

                    if (progressBar->wasCanceled())
                    {
                        return components;
                    }
                }
            }

            return components;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        inline auto
        DcaSolver<ScalarType, EigenSolverType, ProgressBarType>::dual(
            const Matrix<Scalar> &featureMatrix, const PrimalSolution &solution,
            double t) const
        {
            return DualSolution{featureMatrix.transpose() *
                                    (featureMatrix * solution.x),
                                t * solution.u};
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        DcaSolver<ScalarType, EigenSolverType, ProgressBarType>::kktCandidate(
            const Vector<Scalar> &q, const Vector<Scalar> &y, Scalar r) const
        {
            const auto zero = m_Param.zero;
            const auto k = m_Param.k;

            const auto n = q.size();

            DcaSolver::PrimalSolution solution(n);
            auto &x = solution.x;
            auto &u = solution.u;

            const Vector<Scalar> rMinusY = -(y.array() - r);

            const Vector<Scalar> alphar =
                (rMinusY.array() > zero)
                    .select(rMinusY, static_cast<Scalar>(0));

            const Vector<Scalar> deltar =
                (rMinusY.array() < -zero)
                    .select(-rMinusY, static_cast<Scalar>(0));

            const Vector<Scalar> qMinusAlpha = q - alphar;

            const Vector<Scalar> betar =
                (qMinusAlpha.array() < -zero)
                    .select(-qMinusAlpha, static_cast<Scalar>(0));

            const auto twoLambdar =
                (qMinusAlpha.array() > zero)
                    .select(qMinusAlpha, static_cast<Scalar>(0))
                    .norm();

            x = (qMinusAlpha.array() > zero)
                    .select(qMinusAlpha, static_cast<Scalar>(0)) /
                twoLambdar;

            x.normalize();

            Vector<Scalar> oneMinusX = -(x.array() - static_cast<Scalar>(1));

            const auto yEqualRMask =
                (y.array() <= r + zero && y.array() >= r - zero);

            const auto sumOneMinusXWuthRespectYEqualR =
                yEqualRMask.select(oneMinusX, static_cast<Scalar>(0)).sum();

            const Scalar pr = sumOneMinusXWuthRespectYEqualR <= zero
                                  ? static_cast<Scalar>(0)
                                  : (k -
                                     (y.array() <= r + zero)
                                         .select(x, static_cast<Scalar>(0))
                                         .sum() -
                                     (y.array() > r + zero).count()) /
                                        sumOneMinusXWuthRespectYEqualR;

            u = Vector<Scalar>::Ones(n);
            u = (y.array() < r).select(x, u);
            u = yEqualRMask.select(x + pr * oneMinusX, u);

            return solution;
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto
        DcaSolver<ScalarType, EigenSolverType, ProgressBarType>::computePhir(
            const Vector<Scalar> &y, Scalar r,
            const PrimalSolution &solution) const
        {
            const auto zero = m_Param.zero;
            const auto k = m_Param.k;

            const Scalar phir = k -
                                (y.array() <= r + zero)
                                    .select(solution.x, static_cast<Scalar>(0))
                                    .sum() -
                                (y.array() > r + zero).count();

            const auto aMask = (y.array() <= r + zero && y.array() >= r - zero);
            const Scalar rhs =
                aMask.count() -
                aMask.select(solution.x, static_cast<Scalar>(0)).sum();

            return std::pair<Scalar, Scalar>{phir, rhs};
        }

        template <std::floating_point ScalarType,
                  EigenSolverLike EigenSolverType,
                  ProgressBarLike ProgressBarType>
        auto DcaSolver<ScalarType, EigenSolverType, ProgressBarType>::primal(
            [[maybe_unused]] const Matrix<Scalar> &featureMatrix,
            const DualSolution &dualSolution) const
        {
            const auto zero = m_Param.zero;
            const auto k = m_Param.k;

            const Vector<Scalar> q = dualSolution.q.cwiseAbs();
            const auto &y = dualSolution.y;

            const auto n = y.size();

            const auto sortedYIndices = sort(y);

            if (q.norm() <= zero)
            {
                PrimalSolution solution(n);
                solution.u(sortedYIndices).tail(k).setOnes();

                return solution;
            }

            const Vector<Scalar> qPlusY = q + y;

            const auto M = (q.array() > zero)
                               .select(qPlusY, static_cast<Scalar>(0))
                               .maxCoeff();

            const Index Omega = (y.array() >= M - zero).count();
            if (Omega >= k)
            {
                PrimalSolution solution(n);
                solution.u(sortedYIndices).tail(k).setOnes();

                return solution;
            }

            const auto qStrictNonnegAndQPlusYEqualMMask =
                (q.array() > zero && qPlusY.array() >= M - zero &&
                 qPlusY.array() <= M + zero);

            const auto qEqualZeroAndYGreaterThanMMask =
                (q.array() <= zero && y.array() >= M - zero);

            const Index Delta = qStrictNonnegAndQPlusYEqualMMask.count();

            if ((k - Omega) * (k - Omega) <= Delta)
            {
                const auto theta =
                    static_cast<Scalar>(k - Omega) / static_cast<Scalar>(Delta);

                PrimalSolution solution(n);
                auto &x = solution.x;
                auto &u = solution.u;

                x = qStrictNonnegAndQPlusYEqualMMask.select(theta, x);

                u = qStrictNonnegAndQPlusYEqualMMask.select(theta, u);

                u = qEqualZeroAndYGreaterThanMMask.select(
                    static_cast<Scalar>(1), u);

                x.array() *= dualSolution.q.array().sign();

                return solution;
            }

            {
                const auto r = static_cast<Scalar>(0);
                auto solution = kktCandidate(q, y, r);

                const auto [phir, rhs] = computePhir(y, r, solution);

                if (-zero <= phir && phir <= rhs + zero)
                {
                    solution.x.array() *= dualSolution.q.array().sign();

                    return solution;
                }
            }

            auto bm = static_cast<Scalar>(0);
            auto bmPlus1 = static_cast<Scalar>(M);

            for (int j = 0; j < sortedYIndices.size(); ++j)
            {
                const auto yij = y[sortedYIndices[j]];

                if (yij >= M - zero)
                {
                    break;
                }

                auto solution = kktCandidate(q, y, yij);

                const auto [phir, rhs] = computePhir(y, yij, solution);

                if (-zero <= phir)
                {
                    if (phir <= rhs + zero)
                    {
                        solution.x.array() *= dualSolution.q.array().sign();

                        return solution;
                    }

                    bmPlus1 = yij;
                }
                else // if (phir < -zero)
                {
                    bm = yij;
                }
            }

            auto r = static_cast<Scalar>(-1);
            Scalar al = static_cast<Scalar>(0);
            auto qPlusYsortedIndices = sort(qPlusY);
            const auto kMinusSum =
                static_cast<Scalar>(k - (y.array() > bm + zero).count());
            const Scalar kMinusSumSquared = kMinusSum * kMinusSum;
            for (auto l : qPlusYsortedIndices)
            {
                const Scalar alPlus1 = qPlusY[l];

                if (bmPlus1 <= al || alPlus1 <= bm)
                {
                    al = alPlus1;
                    continue;
                }

                const auto minBound = std::max(bm, al);
                const auto maxBound = std::min(bmPlus1, alPlus1);

                const auto iInIlMask =
                    (y.array() <= bm + zero && qPlusY.array() > al + zero);
                const auto sum = static_cast<Scalar>(iInIlMask.count());
                const auto sumQPlusY =
                    iInIlMask.select(qPlusY, static_cast<Scalar>(0)).sum();

                const auto Al = kMinusSumSquared * sum - sum * sum;
                const auto BPrimel = (-kMinusSumSquared + sum) * sumQPlusY;
                const auto Cl =
                    kMinusSumSquared *
                        (iInIlMask
                             .select(qPlusY.cwiseAbs2(), static_cast<Scalar>(0))
                             .sum() +
                         (y.array() > bm + zero)
                             .select(q.cwiseAbs2(), static_cast<Scalar>(0))
                             .sum()) -
                    sumQPlusY * sumQPlusY;

                if (std::abs(Al) <= zero)
                {
                    if (std::abs(BPrimel) > zero)
                    {
                        const auto r0 =
                            -static_cast<Scalar>(0.5) * Cl / BPrimel;
                        if (minBound <= r0 && r0 <= maxBound)
                        {
                            r = r0;
                            break;
                        }
                    }
                }
                else
                {
                    const auto Deltal = BPrimel * BPrimel - Al * Cl;

                    if (std::abs(Deltal) <= zero)
                    {
                        const auto r0 = -BPrimel / Al;

                        if (minBound - zero <= r0 && r0 <= maxBound + zero)
                        {
                            r = r0;
                            break;
                        }
                    }
                    else if (Deltal >= static_cast<Scalar>(0))
                    {
                        const auto sqrtDeltal = std::sqrt(Deltal);
                        const auto r1 = (-BPrimel - sqrtDeltal) / Al;
                        const auto r2 = (-BPrimel + sqrtDeltal) / Al;

                        if (minBound - zero <= r1 && r1 <= maxBound + zero)
                        {
                            r = r1;
                            break;
                        }
                        else if (minBound - zero <= r2 && r2 <= maxBound + zero)
                        {
                            r = r2;
                            break;
                        }
                    }
                }
            }

            if (r >= static_cast<Scalar>(0))
            {
                auto solution = kktCandidate(q, y, r);
                solution.x.array() *= dualSolution.q.array().sign();

                return solution;
            }

            return DcaSolver::PrimalSolution{};
        }

        template <std::floating_point ScalarType>
        using Dca =
            SparsePC<DcaSolver<ScalarType, SpectraLibEigenSolver<ScalarType>>>;
    } // namespace linearmodel
} // namespace Sparsepc
