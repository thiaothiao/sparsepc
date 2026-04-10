#ifndef SPARSEPC_MODEL_HPP
#define SPARSEPC_MODEL_HPP

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#include <QString>
#include <QVector>
#include <QProgressBar>
#include <QLibrary>

#include "sparsepc/core.hpp"

#include "labenums.h"

namespace sparsely
{
    struct MyClass final
    {
        MyClass()
            :iCandidate{-1}, candidates{}
        {
        }

        MyClass(const MyClass&) = default;
        MyClass& operator=(const MyClass&) = default;

        MyClass(MyClass&&) = default;
        MyClass& operator=(MyClass&&) = default;

        int iCandidate;
        std::unordered_map<sparsepc::Index, sparsepc::Component<double>> candidates;
    };

    class LabModel final
    {

    public:
        LabModel();

        bool init(const QString& fileName, bool newProject = true);

        void saveProject(const QString& fileName);

        void loadProject(const QString& fileName);

    public:

        void onSelectionChanged(int index);

    public:
        void computeStandardPCs();
        MyClass& computeSparsePC(const Enums::Method& method, QProgressBar* progressBar);

        bool removeLastSparsePC();

        double computeValidatedCumulativeVariance() const;

        double computeCumulativeVariance() const;

        void validateCurrentSparsePC();

        int getICandidate() const;

        int getCurrentRank() const;

        int getN() const;

        QString getAddonName() const;

        double computeVarianceRatio(double variance) const;

        sparsepc::Matrix<double> m_Sigma;
        sparsepc::Index m_N;
        double m_Trace;


        std::vector<std::reference_wrapper<sparsepc::Component<double>>> m_ValidatedComponents;
        std::vector<MyClass> m_SparsePCs;
        std::vector<MyClass> m_StandardPCs;

        std::vector<std::unique_ptr<QLibrary>> m_DynamicLibSolverLoaders;
        QVector<QString> m_DynamicLibSolverNames;

        //LabWidget& m_LabWidget;
    };
}
#endif //SPARSEPC_MODEL_HPP
