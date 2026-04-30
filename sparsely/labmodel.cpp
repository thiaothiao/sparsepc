#include <labmodel.h>

#include <ranges>
#include <utility>

#include <QLibrary>
#include <QString>

#include <labenums.h>
#include <labprogressdialog.h>

namespace
{
    using BackwardGSPA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::BackwardGspcaSolver<
            double, sparsepc::SpectraLibEigenSolver<double>,
            sparsely::ProgressDialog>>;

    using ForwardGSPCA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::ForwardGspcaSolver<
            double, sparsepc::SpectraLibEigenSolver<double>,
            sparsely::ProgressDialog>>;

    using DCA =
        sparsepc::linearmodel::SparsePC<sparsepc::linearmodel::DcaSolver<
            double, sparsepc::SpectraLibEigenSolver<double>,
            sparsely::ProgressDialog>>;

    using CustomSolver =
        sparsepc::linearmodel::SparsePC<sparsepc::linearmodel::CustomSolver<
            double, sparsepc::SpectraLibEigenSolver<double>,
            sparsely::ProgressDialog>>;

    using DynamicLibSolver =
        sparsepc::linearmodel::SparsePC<sparsepc::linearmodel::DynamicLibSolver<
            double, sparsepc::SpectraLibEigenSolver<double>,
            sparsely::ProgressDialog>>;
} // namespace

namespace sparsely
{
    LabModel::LabModel()
        : m_Sigma{}, m_ValidatedComponents{}, m_SparsePCs{}, m_StandardPCs{},
          m_N{0}, m_Trace{0.0}
    {
    }

    auto LabModel::getStandardComponents() const
    {
        std::vector<std::reference_wrapper<const sparsepc::Component<double>>>
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
        const auto standardComponents = getStandardComponents();
        auto &standardPC = m_StandardPCs.emplace_back();
        standardPC.iWinner = m_N;
        standardPC.candidates = DCA::computeNextComponentCandidates(
            m_Sigma,
            DCA::ImplementationParam{static_cast<sparsepc::Index>(m_N)},
            standardComponents, progressBar);

        for (auto &[k, candidate] : standardPC.candidates)
        {
            candidate.state = sparsepc::ComponentState::Unvalidated;
        }

        auto iter = standardPC.candidates.begin();
        if (iter != standardPC.candidates.end())
        {
            standardPC.iWinner = iter->first;
            iter->second.state = sparsepc::ComponentState::Validated;
        }

        return standardPC;
    }

    Nominees &LabModel::computeNextRoundSparseComponentCandidates(
        const Enums::Method &method, ProgressDialog *progressBar)
    {
        auto &sparsePC = m_SparsePCs.emplace_back();
        sparsePC.iWinner = m_N / 2;
        switch (method)
        {
        case Enums::Method::DCA:
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_Sigma, DCA::ImplementationParam{}, m_ValidatedComponents, progressBar);
            break;
        case Enums::Method::BGSPCA:
            sparsePC.candidates = BackwardGSPA::computeNextComponentCandidates(
                m_Sigma, BackwardGSPA::ImplementationParam{}, m_ValidatedComponents,
                progressBar);
            break;
        case Enums::Method::FGSPCA:
            sparsePC.candidates = ForwardGSPCA::computeNextComponentCandidates(
                m_Sigma, ForwardGSPCA::ImplementationParam{}, m_ValidatedComponents,
                progressBar);
            break;
        case Enums::Method::CUSTOM:
            sparsePC.candidates = CustomSolver::computeNextComponentCandidates(
                m_Sigma, CustomSolver::ImplementationParam{}, m_ValidatedComponents,
                progressBar);
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
                            m_Sigma,
                            DynamicLibSolver::ImplementationParam{
                                computeSparseEigenVector},
                            m_ValidatedComponents, progressBar);
                }
            }
            break;
        }
        default:
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_Sigma, DCA::ImplementationParam{}, m_ValidatedComponents, progressBar);
            break;
        }

        for (auto &[k, candidate] : sparsePC.candidates)
        {
            candidate.state = sparsepc::ComponentState::Unvalidated;
        }

        auto iter = sparsePC.candidates.cbegin();
        if (iter != sparsePC.candidates.cend())
        {
            sparsePC.iWinner = iter->first;
        }

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
                sparsepc::ComponentState::Unvalidated;
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
                sparsepc::ComponentState::Validated;
        }
    }

    double
    LabModel::computePreviousSparseComponentRoundsCumulativeVariance() const
    {
        return std::ranges::fold_left(
            m_ValidatedComponents, 0.0,
            [](double done, const sparsepc::Component<double> &c) {
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
} // namespace sparsely
