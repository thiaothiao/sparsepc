#include <labmodelhandler.h>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QString>
#include <QtLogging>

#include <array>
#include <concepts>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

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

    char findSeparator(const std::string &line)
    {
        static constexpr std::array<char, 4> separators{',', ';', '|', '\t'};
        for (auto separator : separators)
        {
            if (line.find(separator) != std::string::npos)
            {
                qDebug() << "Separator found:" << separator;
                return separator;
            }
        }
        qDebug() << "Separator not found";
        return '\0';
    }

    char findSeparator(std::ifstream &file, int &numberOfColumns)
    {
        std::string line;
        if (std::getline(file, line))
        {
            const auto separator = findSeparator(line);
            if (separator)
            {
                file.clear();
                auto candidateHeader =
                    line | std::views::split(separator) |
                    std::ranges::to<std::vector<std::string>>();
                numberOfColumns = static_cast<int>(candidateHeader.size());
                bool hasHeader = false;
                for (const auto &word : candidateHeader)
                { // assuming at least one non-empty element is not a number
                    if (word.empty())
                    {
                        qDebug() << "Empty element somewhere on the first "
                                    "line of the file";
                        return '\0';
                    }

                    try
                    {
                        [[maybe_unused]] const auto scalarValue =
                            std::stod(word);
                    }
                    catch (...)
                    {
                        hasHeader = true;
                        qDebug() << "Data has header";
                        break;
                    }
                }

                if (!hasHeader && !file.seekg(0, std::ios::beg))
                { // cannot fallback to the beginning of the file. Leave!
                    qDebug() << "Cannot seek at the beginning of the file";
                    return '\0';
                }

                return separator;
            }
        }

        qDebug() << "Cannot seek at the beginning of the file";
        return '\0';
    }

    template <std::floating_point ScalarType>
    sparsepc::Matrix<ScalarType> openData(std::string fileToOpen)
    try
    {
        std::ifstream matrixDataFile(fileToOpen);
        if (matrixDataFile.is_open() && !matrixDataFile.eof())
        { // TODO optimize memory usage
            int matrixColumnNumber = 0;
            const auto separator =
                findSeparator(matrixDataFile, matrixColumnNumber);
            if (!separator)
            {
                // qCritical() << "Cannot find a separator:" << fileToOpen;
                return {};
            }

            using Scalar = ScalarType;
            std::vector<Scalar> matrixEntries;
            std::string matrixRowString;
            std::string matrixEntry;
            int matrixRowNumber = 0;

            while (std::getline(matrixDataFile, matrixRowString))
            {
                std::stringstream matrixRowStringStream(matrixRowString);
                int currentColumnNumber = 0;
                while (
                    std::getline(matrixRowStringStream, matrixEntry, separator))
                {
                    matrixEntries.push_back(
                        static_cast<Scalar>(std::stod(matrixEntry)));
                    ++currentColumnNumber;
                }

                if (matrixColumnNumber != currentColumnNumber)
                {
                    qCritical()
                        << "Non constant column number:" << matrixColumnNumber
                        << "vs" << currentColumnNumber << ":" << fileToOpen;
                    return {};
                }

                if (matrixRowStringStream.bad())
                {
                    qCritical() << "Stream corrupted or hardware failure:"
                                << fileToOpen;
                    return {};
                }

                ++matrixRowNumber;
            }

            if (matrixDataFile.bad())
            {
                qCritical()
                    << "File data corrupted or hardware failure:" << fileToOpen;
                return {};
            }

            if (matrixRowNumber > 0 && matrixColumnNumber > 0 &&
                !matrixEntries.empty())
            {
                const sparsepc::RMMatrix<ScalarType> matrix =
                    Eigen::Map<sparsepc::RMMatrix<Scalar>>(matrixEntries.data(),
                                                           matrixRowNumber,
                                                           matrixColumnNumber);
                qDebug() << "Matrix size:" << matrixRowNumber << "x"
                         << matrixColumnNumber;
                return matrix;
            }
        }

        qCritical() << "Cannot load data:" << fileToOpen;
        return {};
    }
    catch (...)
    {
        qCritical() << "An exception pops up:" << fileToOpen;
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
        qDebug() << "Initializing model handler...";
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
                    qCritical() << "Load data as matrix failed:" << fileName;
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

        qDebug() << "...model handler initialized";
        return true;
    }

    bool ModelHandler::saveProject(const QString &fileName) const
    {
        qDebug() << "Saving project...";
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            qCritical() << "Cannot open file for write only:" << fileName;
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
        qDebug() << "... project saved";
        return true;
    }

    bool ModelHandler::loadProject(const QString &fileName)
    {
        qDebug() << "Loading project...";
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            qCritical() << "Cannot open file for read only:" << fileName;
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
        qDebug() << "... project loaded";
        return true;
    }

    void ModelHandler::loadAddon(const QString &path)
    {
        auto &dynamicLibSolverLoaders = m_Model.get().m_DynamicLibSolverLoaders;
        auto &dynamicLibSolverNames = m_Model.get().m_DynamicLibSolverNames;
        qDebug() << "Loading addon ...";
        if (!dynamicLibSolverLoaders.empty())
        {
            qDebug() << "Addon already loaded";
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

            qDebug() << "... addon loaded";
        }
        else
        {
            qDebug() << "No plugin";
        }
    }
} // namespace sparsely
