#include <labmodel.h>

#include <ranges>
#include <utility>

#include <QLibrary>
#include <QString>
#include <QtLogging>

#include <labenums.h>
#include <labprogressdialog.h>

namespace
{
    using BackwardGSPA = Sparsepc::linearmodel::SparsePC<
        Sparsepc::linearmodel::BackwardGspcaSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;

    using ForwardGSPCA = Sparsepc::linearmodel::SparsePC<
        Sparsepc::linearmodel::ForwardGspcaSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;

    using DCA =
        Sparsepc::linearmodel::SparsePC<Sparsepc::linearmodel::DcaSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;

    using CustomSolver =
        Sparsepc::linearmodel::SparsePC<Sparsepc::linearmodel::CustomSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;

    using DynamicLibSolver =
        Sparsepc::linearmodel::SparsePC<Sparsepc::linearmodel::DynamicLibSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;

    using ContiguousSupportSolver = Sparsepc::linearmodel::SparsePC<
        Sparsepc::linearmodel::ContiguousSupportSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;

    using MavIterativeEliminationSolver = Sparsepc::linearmodel::SparsePC<
        Sparsepc::linearmodel::MavIterativeEliminationSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;

    using AmvlIterativeEliminationSolver = Sparsepc::linearmodel::SparsePC<
        Sparsepc::linearmodel::AmvlIterativeEliminationSolver<
            double, Sparsepc::SpectraLibEigenSolver<double>,
            Sparsely::ProgressDialog>>;
} // namespace

namespace Sparsely
{
    LabModel::LabModel()
        : m_FeatureMatrix{}, m_ValidatedComponents{}, m_SparsePCs{},
          m_StandardPCs{}, m_N{0}, m_Trace{0.0}
    {
    }

    auto LabModel::getStandardComponents() const
    {
        std::vector<std::reference_wrapper<const Sparsepc::Component<double>>>
            standardComponents;
        standardComponents.reserve(m_StandardPCs.size());
        for (const auto &standardPC : m_StandardPCs)
        {
            standardComponents.push_back(
                standardPC.candidates.at(standardPC.iWinner));
        }

        return standardComponents;
    }

    Nominees &
    LabModel::computeNextRoundStandardComponent(ProgressDialog *progressBar)
    {
        qDebug() << "Computing next round standard component ...";
        const auto standardComponents = getStandardComponents();
        auto &standardPC = m_StandardPCs.emplace_back();
        standardPC.iWinner = m_N;
        standardPC.candidates = DCA::computeNextComponentCandidates(
            m_FeatureMatrix,
            DCA::ImplementationParam{static_cast<Sparsepc::Index>(m_N)},
            standardComponents, progressBar);

        for (auto &[k, candidate] : standardPC.candidates)
        {
            candidate.state = Sparsepc::ComponentState::Unvalidated;
        }

        auto iter = standardPC.candidates.begin();
        if (iter != standardPC.candidates.end())
        {
            standardPC.iWinner = iter->first;
            iter->second.state = Sparsepc::ComponentState::Validated;
        }

        qDebug() << "... next round standard component computed";

        return standardPC;
    }

    Nominees &LabModel::computeNextRoundSparseComponentCandidates(
        const Enums::Method &method, ProgressDialog *progressBar)
    {
        qDebug() << "Computing next round sparse component candidates ...";
        auto &sparsePC = m_SparsePCs.emplace_back();
        sparsePC.iWinner = m_N / 2;
        switch (method)
        {
        case Enums::Method::DCA:
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_FeatureMatrix, DCA::ImplementationParam{},
                m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::BGSPCA:
            sparsePC.candidates = BackwardGSPA::computeNextComponentCandidates(
                m_FeatureMatrix, BackwardGSPA::ImplementationParam{},
                m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::FGSPCA:
            sparsePC.candidates = ForwardGSPCA::computeNextComponentCandidates(
                m_FeatureMatrix, ForwardGSPCA::ImplementationParam{},
                m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::CUSTOM:
            sparsePC.candidates = CustomSolver::computeNextComponentCandidates(
                m_FeatureMatrix, CustomSolver::ImplementationParam{},
                m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::CONTIGUOUSSUPPORT:
            sparsePC.candidates =
                ContiguousSupportSolver::computeNextComponentCandidates(
                    m_FeatureMatrix,
                    ContiguousSupportSolver::ImplementationParam{},
                    m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::MAVIEA:
            sparsePC.candidates =
                MavIterativeEliminationSolver::computeNextComponentCandidates(
                    m_FeatureMatrix,
                    MavIterativeEliminationSolver::ImplementationParam{},
                    m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::AMVLIEA:
            sparsePC.candidates =
                AmvlIterativeEliminationSolver::computeNextComponentCandidates(
                    m_FeatureMatrix,
                    AmvlIterativeEliminationSolver::ImplementationParam{},
                    m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::USERDYNAMICLIB: {
            auto &library = m_DynamicLibSolverLoaders.at(0);
            if (library)
            {
                auto computeSparseEigenVector =
                    (ComputeSparseEigenVector)library->resolve(
                        "computeSparseEigenVector");
                if (computeSparseEigenVector)
                {
                    sparsePC.candidates =
                        DynamicLibSolver::computeNextComponentCandidates(
                            m_FeatureMatrix,
                            DynamicLibSolver::ImplementationParam{
                                computeSparseEigenVector},
                            m_ValidatedComponents, progressBar);
                }
            }
            break;
        }
        default:
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_FeatureMatrix, DCA::ImplementationParam{},
                m_ValidatedComponents, progressBar);
            break;
        }

        for (auto &[k, candidate] : sparsePC.candidates)
        {
            candidate.state = Sparsepc::ComponentState::Unvalidated;
        }

        auto iter = sparsePC.candidates.cbegin();
        if (iter != sparsePC.candidates.cend())
        {
            sparsePC.iWinner = iter->first;
        }

        qDebug() << "... next round sparse component candidates computed";

        return sparsePC;
    }

    bool LabModel::removeLastStandardComponent()
    {
        if (m_StandardPCs.empty())
        {
            return false;
        }

        m_StandardPCs.pop_back();

        return true;
    }

    bool LabModel::removeLastSparsePC()
    {
        if (m_SparsePCs.empty())
        {
            return false;
        }

        if (!m_ValidatedComponents.empty())
        {
            m_ValidatedComponents.back().get().state =
                Sparsepc::ComponentState::Unvalidated;
            m_ValidatedComponents.pop_back();
        }
        m_SparsePCs.pop_back();

        return true;
    }

    void LabModel::validateCurrentSparseComponent()
    {
        if (!m_SparsePCs.empty())
        { // validate current selected sparse component
            auto &candidates = m_SparsePCs.back().candidates;
            const auto iWinner = m_SparsePCs.back().iWinner;

            m_ValidatedComponents.push_back(candidates.at(iWinner));
            m_ValidatedComponents.back().get().state =
                Sparsepc::ComponentState::Validated;
            qDebug() << " Sparse component candidate validated";
        }
    }

    double
    LabModel::computePreviousSparseComponentRoundsCumulativeVariance() const
    {
        return std::ranges::fold_left(
            m_ValidatedComponents, 0.0,
            [](double done, const Sparsepc::Component<double> &c) {
                return done + c.value;
            });
    }

    double LabModel::computeStandardComponentsCumulativeVariance() const
    {
        return std::ranges::fold_left(
            m_StandardPCs, 0.0, [](double done, const Nominees &c) {
                return done + c.candidates.at(c.iWinner).value;
            });
    }

    int LabModel::getiWinner() const
    {
        return m_SparsePCs.empty() ? 1 : m_SparsePCs.back().iWinner;
    }

    int LabModel::getCurrentSparseComponentRound() const
    {
        return m_ValidatedComponents.size();
    }

    int LabModel::getN() const { return m_N; }

    QString LabModel::getAddonName() const
    {
        return m_DynamicLibSolverNames.empty() ? ""
                                               : m_DynamicLibSolverNames.at(0);
    }

    double LabModel::computeVarianceRatio(double variance) const
    {
        return variance / m_Trace;
    }
} // namespace Sparsely
