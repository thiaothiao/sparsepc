#include <labmodel.h>

#include <ranges>
#include <utility>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QString>

#include <labprogressdialog.h>

namespace
{
    using BackwardGSPA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::BackwardGspcaModel<
            double, sparsepc::EigenSolver<double>, sparsely::ProgressDialog>>;

    using ForwardGSPCA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::ForwardGspcaModel<
            double, sparsepc::EigenSolver<double>, sparsely::ProgressDialog>>;

    using DCA = sparsepc::linearmodel::SparsePC<sparsepc::linearmodel::DcaModel<
        double, sparsepc::EigenSolver<double>, sparsely::ProgressDialog>>;

    using CustomSolver = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::CustomSolverModel<
            double, sparsepc::EigenSolver<double>, sparsely::ProgressDialog>>;

    using DynamicLibSolver = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::DynamicLibSolverModel<
            double, sparsepc::EigenSolver<double>, sparsely::ProgressDialog>>;

    auto getPluginList(const QString &path)
    {
        const QDir dir(path);
        QStringList filters;
        filters << "*.dll" << "*.so" << "*.dylib";
        return dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    }
} // namespace

namespace sparsely
{
    LabModel::LabModel()
        : m_Sigma{}, m_ValidatedComponents{}, m_SparsePCs{}, m_StandardPCs{},
          m_N{0}, m_Trace{0.0}
    {
    }

    bool LabModel::init(const QString &fileName, bool newProject)
    {
        if (newProject)
        {
            { // TODO improve covariance computations
                const sparsepc::Matrix<double> X =
                    sparsepc::openData<double>(fileName.toStdString(), ';');

                if (X.rows() <= 1)
                {
                    // TODO should popup one sample matrix.
                    return false;
                }

                const sparsepc::Matrix<double> centeredX =
                    X.rowwise() - X.colwise().mean();

                // sample covariance formula
                m_Sigma = (centeredX.adjoint() * centeredX) /
                          static_cast<double>(X.rows() - 1);
            }

            m_N = m_Sigma.cols();
            m_Trace = m_Sigma.trace();
            m_StandardPCs.clear();
            m_StandardPCs.reserve(m_N);
            m_SparsePCs.clear();
            m_SparsePCs.reserve(m_N);
            m_ValidatedComponents.clear();
            m_ValidatedComponents.reserve(m_N);
        }
        else
        {
            loadProject(fileName);
        }

        return true;
    }

    sparsepc::Component<double> &
    LabModel::computeStandardPC(ProgressDialog *progressBar)
    { // TODO really update the progress during computations
        using Index = sparsepc::Index;
        const DCA::Param param{std::vector<DCA::ModelParam>(
            m_StandardPCs.size() + 1, {static_cast<Index>(m_N)})};
        auto components = DCA{param}.run(m_Sigma);
        m_StandardPCs.emplace_back();
        auto &standardPC = m_StandardPCs.back();
        standardPC.iWinner = m_N;
        components.back().state = sparsepc::ComponentState::Validated;
        return standardPC.candidates.emplace(m_N, std::move(components.back()))
            .first->second;
    }

    Nominees &LabModel::computeSparsePC(const Enums::Method &method,
                                        ProgressDialog *progressBar)
    {
        auto &sparsePC = m_SparsePCs.emplace_back();
        sparsePC.iWinner = m_N / 2;
        switch (method)
        {
        case Enums::Method::DCA:
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_Sigma, DCA::ModelParam(1), m_ValidatedComponents,
                progressBar);
            break;
        case Enums::Method::BGSPCA:
            sparsePC.candidates = BackwardGSPA::computeNextComponentCandidates(
                m_Sigma, BackwardGSPA::ModelParam(1), m_ValidatedComponents,
                progressBar);
            break;
        case Enums::Method::FGSPCA:
            sparsePC.candidates = ForwardGSPCA::computeNextComponentCandidates(
                m_Sigma, ForwardGSPCA::ModelParam(1), m_ValidatedComponents,
                progressBar);
            break;
        case Enums::Method::CUSTOM:
            sparsePC.candidates = CustomSolver::computeNextComponentCandidates(
                m_Sigma, CustomSolver::ModelParam(1), m_ValidatedComponents,
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
                            DynamicLibSolver::ModelParam(
                                computeSparseEigenVector),
                            m_ValidatedComponents, progressBar);
                }
            }
            break;
        }
        default:
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_Sigma, DCA::ModelParam(1), m_ValidatedComponents,
                progressBar);
            break;
        }

        for (auto &[k, candidate] : sparsePC.candidates)
        {
            candidate.state = sparsepc::ComponentState::Unvalidated;
        }

        return sparsePC;
    }

    bool LabModel::removeLastStandardPC()
    {
        if (m_StandardPCs.size() <= 2)
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

    void LabModel::validateCurrentSparsePC()
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

    double LabModel::computeValidatedCumulativeVariance() const
    {
        return std::ranges::fold_left(
            m_ValidatedComponents, 0.0,
            [](double done, const sparsepc::Component<double> &c) {
                return done + c.value;
            });
    }

    double LabModel::computeStandardCumulativeVariance() const
    {
        return std::ranges::fold_left(
            m_StandardPCs, 0.0, [](double done, const Nominees &c) {
                return done + c.candidates.at(c.iWinner).value;
            });
    }

    double LabModel::computeCumulativeVariance() const
    {
        return m_SparsePCs.empty()
                   ? computeValidatedCumulativeVariance()
                   : (computeValidatedCumulativeVariance() +
                      m_SparsePCs.back()
                          .candidates.at(m_SparsePCs.back().iWinner)
                          .value);
    }

    int LabModel::getiWinner() const
    {
        return m_SparsePCs.empty() ? 1 : m_SparsePCs.back().iWinner;
    }

    int LabModel::getCurrentRank() const
    {
        return m_ValidatedComponents.size();
    }

    int LabModel::getN() const { return m_N; }

    QString LabModel::getAddonName() const
    {
        return m_DynamicLibSolverNames.empty() ? ""
                                               : m_DynamicLibSolverNames.at(0);
    }

    void LabModel::loadAddon(const QString &path)
    {
        if (!m_DynamicLibSolverLoaders.empty())
        {
            return;
        }

        const auto pluginList = getPluginList(path);
        if (!pluginList.empty())
        {
            m_DynamicLibSolverLoaders.reserve(pluginList.size());
            m_DynamicLibSolverNames.reserve(pluginList.size());
            foreach (const auto &fileInfo, pluginList)
            {
                QString noExtensionAbsolutePath =
                    QDir(fileInfo.absolutePath()).filePath(fileInfo.baseName());
                m_DynamicLibSolverLoaders.push_back(
                    std::make_unique<QLibrary>(noExtensionAbsolutePath));
                if (m_DynamicLibSolverLoaders.back()->load())
                {
                    m_DynamicLibSolverNames.push_back(fileInfo.baseName());
                }
                else
                {
                    m_DynamicLibSolverLoaders.pop_back();
                }
            }
        }
    }

    double LabModel::computeVarianceRatio(double variance) const
    {
        return variance / m_Trace;
    }

    // write operator
    QDataStream &operator<<(QDataStream &out, const Nominees &user)
    {
        out << static_cast<qint32>(user.iWinner);
        out << static_cast<qint32>(user.candidates.size());
        for (auto &[k, component] : user.candidates)
        {
            out << static_cast<qint32>(k);
            out << static_cast<qint32>(std::to_underlying(component.state));
            out << static_cast<double>(component.value);
            out << static_cast<qint32>(component.vector.size());
            out.writeRawData(
                reinterpret_cast<const char *>(component.vector.data()),
                component.vector.size() * sizeof(double));
            out << static_cast<qint32>(component.q.size());
            if (component.q.size() != 0)
            {
                out.writeRawData(
                    reinterpret_cast<const char *>(component.q.data()),
                    component.q.size() * sizeof(double));
            }
        }
        return out;
    }

    // read operator
    QDataStream &operator>>(QDataStream &in, Nominees &user)
    {
        in >> user.iWinner;
        qint32 candidatesSize = -1;
        in >> candidatesSize;
        for (qint32 j = 0; j < candidatesSize; ++j)
        {
            qint32 valInt = -1;
            in >> valInt;
            auto &component = user.candidates[valInt];
            in >> valInt;
            component.state = static_cast<sparsepc::ComponentState>(valInt);
            auto val = static_cast<double>(-1);
            in >> val;
            component.value = val;
            in >> valInt;
            component.vector = sparsepc::Vector<double>(valInt);
            in.readRawData(reinterpret_cast<char *>(component.vector.data()),
                           valInt * sizeof(double));

            in >> valInt;
            if (valInt != 0)
            {
                component.q = sparsepc::Vector<double>(valInt);
                in.readRawData(reinterpret_cast<char *>(component.q.data()),
                               valInt * sizeof(double));
            }
        }

        return in;
    }

    bool LabModel::saveProject(const QString &filename) const
    {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }

        // Serialization
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0); // version for forward/backward
                                             // compatibility
        out << static_cast<qint32>(m_N);
        out.writeRawData(reinterpret_cast<const char *>(m_Sigma.data()),
                         m_N * m_N * sizeof(double));
        const auto standardPCsSize = static_cast<qint32>(m_StandardPCs.size());
        out << standardPCsSize;
        for (qint32 j = 0; j < standardPCsSize; ++j)
        {
            out << m_StandardPCs[j];
        }
        const auto sparsePCsSize = static_cast<qint32>(m_SparsePCs.size());
        out << sparsePCsSize;
        for (qint32 j = 0; j < sparsePCsSize; ++j)
        {
            out << m_SparsePCs[j];
        }
        file.close();
        return true;
    }

    bool LabModel::loadProject(const QString &fileName)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        // Deserialization
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);
        qint32 intVal = -1;
        in >> intVal;
        m_N = intVal;
        m_Sigma.resize(m_N, m_N);
        in.readRawData(reinterpret_cast<char *>(m_Sigma.data()),
                       m_N * m_N * sizeof(double));
        m_Trace = m_Sigma.trace();
        m_StandardPCs.clear();
        m_StandardPCs.reserve(m_N);
        m_SparsePCs.clear();
        m_SparsePCs.reserve(m_N);
        in >> intVal;
        for (qint32 j = 0; j < intVal; ++j)
        {
            Nominees Nominees;
            in >> Nominees;
            m_StandardPCs.push_back(std::move(Nominees));
        }
        in >> intVal;
        for (qint32 j = 0; j < intVal; ++j)
        {
            Nominees Nominees;
            in >> Nominees;
            m_SparsePCs.push_back(std::move(Nominees));
        }
        file.close();
        return true;
    }
} // namespace sparsely
