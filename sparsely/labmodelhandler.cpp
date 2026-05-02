#include <labmodelhandler.h>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QString>

#include <string>

#include <labmodel.h>

#include <sparsepc/core.hpp>

namespace
{
    auto getPluginList(const QString &path)
    {
        const QDir dir(path);
        QStringList filters;
        filters << "*.dll" << "*.so" << "*.dylib";
        return dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    }

    char delimiteridentifier(const std::string &line)
    {
        constexpr std::array<char, 4> delimiters{',', ';', '|', '\t'};
        for (auto delimiter : delimiters)
        {
            if (line.find(delimiter) != std::string::npos)
            {
                return delimiter;
            }
        }
        return '\0';
    }

    char delimiteridentifier(std::ifstream &file)
    {
        std::string line;
        if (std::getline(file, line))
        {
            const auto delimiter = delimiteridentifier(line);
            file.clear();
            if (file.seekg(0, std::ios::beg))
            {
                return delimiter;
            }
        }

        return '\0';
    }

    template <std::floating_point ScalarType>
    sparsepc::Matrix<ScalarType> openData(std::string fileToOpen)
    try
    {
        std::ifstream matrixDataFile(fileToOpen);
        if (matrixDataFile.is_open() && !matrixDataFile.eof())
        { // TODO optimize memory usage
            const auto sep = delimiteridentifier(matrixDataFile);
            if (!sep)
            {
                std::cout << "Fatal error: Cannot identify separator."
                          << std::endl;
                return {};
            }

            using Scalar = ScalarType;
            std::vector<Scalar> matrixEntries;
            std::string matrixRowString;
            std::string matrixEntry;
            int matrixRowNumber = 0;
            int matrixColumnNumber = 0;
            bool columnNumberInitialized = false;

            while (std::getline(matrixDataFile, matrixRowString))
            {
                std::stringstream matrixRowStringStream(matrixRowString);
                int currentColumnNumber = 0;
                while (std::getline(matrixRowStringStream, matrixEntry, sep))
                {
                    matrixEntries.push_back(
                        static_cast<Scalar>(std::stod(matrixEntry)));
                    ++currentColumnNumber;
                }

                if (columnNumberInitialized)
                {
                    if (matrixColumnNumber != currentColumnNumber)
                    {
                        std::cout << "Fatal error: Non constant column number."
                                  << std::endl;
                        return {};
                    }
                }
                else
                {
                    matrixColumnNumber = currentColumnNumber;
                    columnNumberInitialized = true;
                }

                if (matrixRowStringStream.bad())
                {
                    std::cout << "Fatal error: Stream corrupted or "
                                 "hardware failure."
                              << std::endl;
                    return {};
                }

                ++matrixRowNumber;
            }

            if (matrixDataFile.bad())
            {
                std::cout << "Fatal error: file data corrupted or "
                             "hardware failure."
                          << std::endl;
                return {};
            }

            if (matrixRowNumber > 0 && matrixColumnNumber > 0 &&
                !matrixEntries.empty())
            {
                const sparsepc::RMMatrix<ScalarType> matrix =
                    Eigen::Map<sparsepc::RMMatrix<Scalar>>(matrixEntries.data(),
                                                           matrixRowNumber,
                                                           matrixColumnNumber);
                return matrix;
            }
        }

        return {};
    }
    catch (...)
    {
        std::cout << "An exception pops up!!!" << std::endl;
        return {};
    }

    // write operator
    QDataStream &operator<<(QDataStream &out, const sparsely::Nominees &user)
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
    QDataStream &operator>>(QDataStream &in, sparsely::Nominees &user)
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

} // namespace

namespace sparsely
{
    ModelHandler::ModelHandler(LabModel &modelToBuild) : m_Model{modelToBuild}
    {
    }

    bool ModelHandler::init(const QString &fileName, bool newProject)
    {
        auto &n = m_Model.get().m_N;
        auto &sigma = m_Model.get().m_Sigma;
        auto &trace = m_Model.get().m_Trace;
        auto &standardPCs = m_Model.get().m_StandardPCs;
        auto &sparsePCs = m_Model.get().m_SparsePCs;
        auto &validatedComponents = m_Model.get().m_ValidatedComponents;

        if (newProject)
        {
            { // TODO improve covariance computations
                const sparsepc::Matrix<double> X =
                    openData<double>(fileName.toStdString());

                if (X.rows() <= 1)
                {
                    // TODO should popup one sample matrix.
                    return false;
                }

                const sparsepc::Matrix<double> centeredX =
                    X.rowwise() - X.colwise().mean();

                // sample covariance formula
                sigma = (centeredX.adjoint() * centeredX) /
                        static_cast<double>(X.rows() - 1);
            }

            n = sigma.cols();
            trace = sigma.trace();
            standardPCs.clear();
            standardPCs.reserve(n);
            sparsePCs.clear();
            sparsePCs.reserve(n);
            validatedComponents.clear();
            validatedComponents.reserve(n);
        }
        else
        {
            loadProject(fileName);
        }

        return true;
    }

    bool ModelHandler::saveProject(const QString &fileName) const
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }

        const auto &n = m_Model.get().m_N;
        const auto &sigma = m_Model.get().m_Sigma;
        const auto &standardPCs = m_Model.get().m_StandardPCs;
        const auto &sparsePCs = m_Model.get().m_SparsePCs;

        // Serialization
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0); // version for forward/backward
                                             // compatibility
        out << static_cast<qint32>(n);
        out.writeRawData(reinterpret_cast<const char *>(sigma.data()),
                         n * n * sizeof(double));
        const auto standardPCsSize = static_cast<qint32>(standardPCs.size());
        out << standardPCsSize;
        for (qint32 j = 0; j < standardPCsSize; ++j)
        {
            out << standardPCs[j];
        }
        const auto sparsePCsSize = static_cast<qint32>(sparsePCs.size());
        out << sparsePCsSize;
        for (qint32 j = 0; j < sparsePCsSize; ++j)
        {
            out << sparsePCs[j];
        }
        file.close();
        return true;
    }

    bool ModelHandler::loadProject(const QString &fileName)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        auto &n = m_Model.get().m_N;
        auto &sigma = m_Model.get().m_Sigma;
        auto &trace = m_Model.get().m_Trace;
        auto &standardPCs = m_Model.get().m_StandardPCs;
        auto &sparsePCs = m_Model.get().m_SparsePCs;

        // Deserialization
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);
        qint32 intVal = -1;
        in >> intVal;
        n = intVal;
        sigma.resize(n, n);
        in.readRawData(reinterpret_cast<char *>(sigma.data()),
                       n * n * sizeof(double));
        trace = sigma.trace();
        standardPCs.clear();
        standardPCs.reserve(n);
        sparsePCs.clear();
        sparsePCs.reserve(n);
        in >> intVal;
        for (qint32 j = 0; j < intVal; ++j)
        {
            Nominees Nominees;
            in >> Nominees;
            standardPCs.push_back(std::move(Nominees));
        }
        in >> intVal;
        for (qint32 j = 0; j < intVal; ++j)
        {
            Nominees Nominees;
            in >> Nominees;
            sparsePCs.push_back(std::move(Nominees));
        }
        file.close();
        return true;
    }

    void ModelHandler::loadAddon(const QString &path)
    {
        auto &dynamicLibSolverLoaders = m_Model.get().m_DynamicLibSolverLoaders;
        auto &dynamicLibSolverNames = m_Model.get().m_DynamicLibSolverNames;

        if (!dynamicLibSolverLoaders.empty())
        {
            return;
        }

        const auto pluginList = getPluginList(path);
        if (!pluginList.empty())
        {
            dynamicLibSolverLoaders.reserve(pluginList.size());
            dynamicLibSolverNames.reserve(pluginList.size());
            foreach (const auto &fileInfo, pluginList)
            {
                QString noExtensionAbsolutePath =
                    QDir(fileInfo.absolutePath()).filePath(fileInfo.baseName());
                dynamicLibSolverLoaders.push_back(
                    std::make_unique<QLibrary>(noExtensionAbsolutePath));
                if (dynamicLibSolverLoaders.back()->load())
                {
                    dynamicLibSolverNames.push_back(fileInfo.baseName());
                }
                else
                {
                    dynamicLibSolverLoaders.pop_back();
                }
            }
        }
    }
} // namespace sparsely
