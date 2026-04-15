#ifndef SPARSEPC_MODEL_HPP
#define SPARSEPC_MODEL_HPP

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include <QLibrary>
#include <QProgressBar>
#include <QString>
#include <QVector>

#include "sparsepc/core.hpp"

#include "labenums.h"

namespace sparsely
{
    struct Nominees final
    {
        Nominees() : iWinner{-1}, candidates{} {}
        Nominees(const Nominees &) = default;
        Nominees &operator=(const Nominees &) = default;
        Nominees(Nominees &&) = default;
        Nominees &operator=(Nominees &&) = default;

        int iWinner;
        std::unordered_map<sparsepc::Index, sparsepc::Component<double>>
            candidates;
    };

    class LabModel final
    {
      public:
        LabModel();
        bool init(const QString &fileName, bool newProject = true);
        void saveProject(const QString &fileName) const;
        void loadProject(const QString &fileName);
        void computeStandardPCs();
        Nominees &computeSparsePC(const Enums::Method &method,
                                  QProgressBar *progressBar);
        bool removeLastSparsePC();
        double computeValidatedCumulativeVariance() const;
        double computeCumulativeVariance() const;
        void validateCurrentSparsePC();
        int getiWinner() const;
        int getCurrentRank() const;
        int getN() const;
        QString getAddonName() const;
        double computeVarianceRatio(double variance) const;

        std::vector<std::reference_wrapper<sparsepc::Component<double>>>
            m_ValidatedComponents;
        std::vector<Nominees> m_SparsePCs;
        std::vector<Nominees> m_StandardPCs;

      private:
        sparsepc::Matrix<double> m_Sigma;
        sparsepc::Index m_N;
        double m_Trace;
        std::vector<std::unique_ptr<QLibrary>> m_DynamicLibSolverLoaders;
        QVector<QString> m_DynamicLibSolverNames;
    };
} // namespace sparsely
#endif // SPARSEPC_MODEL_HPP
