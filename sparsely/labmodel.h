#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include <QLibrary>
#include <QString>
#include <QVector>

#include <Sparsepc/core.hpp>

class QLibrary;

namespace Sparsely
{
    class ProgressDialog;
    namespace Enums
    {
        enum class Method : uint8_t;
    }

    // Represents a set of sparse candidates and a selected one on a given
    // principal component round.
    struct Nominees final
    {
        Nominees() : iWinner{-1}, candidates{} {}
        Nominees(const Nominees &) = default;
        Nominees &operator=(const Nominees &) = default;
        Nominees(Nominees &&) = default;
        Nominees &operator=(Nominees &&) = default;

        int iWinner; // The selected candidate location.
        std::unordered_map<Sparsepc::Index, Sparsepc::Component<double>>
            candidates; // The candidates container.
    };

    // The model part of the MVC.
    // It encapsulates processings and data handles.
    class LabModel final
    {
      public:
        friend class ModelHandler;

        LabModel();

        // launch next round standard component computations
        Nominees &
        computeNextRoundStandardComponent(ProgressDialog *progressBar);
        // launch next round sparse component candidates computations
        Nominees &
        computeNextRoundSparseComponentCandidates(const Enums::Method &method,
                                                  ProgressDialog *progressBar);
        // remove last computed standard component
        // current standard component round component
        bool removeLastStandardComponent();
        // remove last computed sparse components
        // all candidates from current sparse component round
        bool removeLastSparsePC();
        // return previous rounds sparse components cumulative explained
        // variances
        double computePreviousSparseComponentRoundsCumulativeVariance() const;
        // return standard components cumulative explained variances
        double computeStandardComponentsCumulativeVariance() const;
        // Validate current sparse component as the winner among the nominees.
        void validateCurrentSparseComponent();
        // return the winning sparse component location among the nominees
        int getiWinner() const;
        // return current sparse component round
        int getCurrentSparseComponentRound() const;
        // return the ratio with respect to covariance matrix total variance
        double computeVarianceRatio(double variance) const;
        int getN() const;
        QString getAddonName() const;

        std::vector<std::reference_wrapper<Sparsepc::Component<double>>>
            m_ValidatedComponents;
        std::vector<Nominees> m_SparsePCs;
        std::vector<Nominees> m_StandardPCs;

      private:
        auto getStandardComponents() const;

        Sparsepc::Matrix<double> m_Sigma;
        Sparsepc::Index m_N;
        double m_Trace;
        std::vector<std::unique_ptr<QLibrary>> m_DynamicLibSolverLoaders;
        QVector<QString> m_DynamicLibSolverNames;
    };
} // namespace Sparsely
