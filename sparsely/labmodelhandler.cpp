#include <labmodelhandler.h>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QString>
#include <QtLogging>

#include <algorithm>
#include <array>
#include <cctype>
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
    constexpr qint32 magicNumber = -1;
    constexpr qint32 versionNumber = 1;

    auto getPluginList(const QString &path)
    {
        const QDir dir(path);
        QStringList filters;
        filters << "*.dll" << "*.so" << "*.dylib";
        return dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    }

    bool hasOnlySpaces(const std::string &s)
    {
        return std::count_if(s.begin(), s.end(), [](unsigned char c) {
                   return std::isspace(c) != 0;
               }) == s.size();
    }

    char findSeparator(const std::string &line)
    {
        static constexpr std::array<char, 3> separators{',', ';', '|'};
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

    char findSeparator(std::ifstream &file, int &numberOfColumns,
                       QVector<QString> &candidateHeader)
    {
        std::string line;
        if (std::getline(file, line))
        {
            const auto separator = findSeparator(line);
            if (separator)
            {
                file.clear();
                // const auto candidateHeader =
                //     line | std::views::split(separator) |
                //     std::ranges::to<std::vector<std::string>>(); waiting for
                //     c++23 on gcc 14.1+

                auto split_view = line | std::views::split(separator);
                candidateHeader.clear();
                for (auto &&part : split_view)
                {
                    // safe but less efficient
                    candidateHeader.push_back(QString::fromStdString(
                        std::string(std::string_view(part))));
                }
                numberOfColumns = static_cast<int>(candidateHeader.size());
                bool hasHeader = false;
                for (const auto &word : candidateHeader)
                { // assuming at least one non-empty element is not a number
                    if (word.isEmpty())
                    {
                        qDebug() << "Empty element somewhere on the first "
                                    "line of the file";
                        return '\0';
                    }

                    bool ok{};
                    [[maybe_unused]] const auto value =
                        word.trimmed().toDouble(&ok);
                    if (!ok)
                    {
                        hasHeader = true;
                        qDebug() << "Data has header";
                        break;
                    }
                }

                if (!hasHeader)
                {
                    candidateHeader.clear(); // no header
                    for (int i = 0; i < numberOfColumns; ++i)
                    {
                        candidateHeader.push_back(QString::number(i));
                    }
                    if (!file.seekg(0, std::ios::beg))
                    { // cannot fallback to the beginning of the file. Leave!
                        qDebug() << "Cannot seek at the beginning of the file";
                        return '\0';
                    }
                }

                return separator;
            }
        }

        qDebug() << "Cannot seek at the beginning of the file";
        return '\0';
    }

    template <std::floating_point ScalarType>
    Sparsepc::Matrix<ScalarType> openData(std::string fileToOpen,
                                          QVector<QString> &candidateHeader)
    {
        std::ifstream matrixDataFile(fileToOpen);
        if (matrixDataFile.is_open() && !matrixDataFile.eof())
        { // TODO optimize memory usage
            int matrixColumnNumber = 0;
            const auto separator = findSeparator(
                matrixDataFile, matrixColumnNumber, candidateHeader);
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
                    bool ok{};
                    const auto scalarValue = QString::fromStdString(matrixEntry)
                                                 .trimmed()
                                                 .toDouble(&ok);
                    if (!ok)
                    {
                        qCritical() << "Cannot convert string: " << matrixEntry;
                        return {};
                    }

                    matrixEntries.push_back(static_cast<Scalar>(scalarValue));
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
                const Sparsepc::RMMatrix<ScalarType> matrix =
                    Eigen::Map<Sparsepc::RMMatrix<Scalar>>(matrixEntries.data(),
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

    // write operator
    QDataStream &operator<<(QDataStream &out, const Sparsely::Nominees &user)
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
    QDataStream &operator>>(QDataStream &in, Sparsely::Nominees &user)
    {
        qint32 tmp = -1;
        in >> tmp;
        user.iWinner = tmp;

        in >> tmp;
        const qint32 candidatesSize = tmp;
        for (qint32 j = 0; j < candidatesSize; ++j)
        {
            in >> tmp;
            auto &component = user.candidates[tmp];
            in >> tmp;
            component.state = static_cast<Sparsepc::ComponentState>(tmp);
            double val = -1.0;
            in >> val;
            component.value = val;
            in >> tmp;
            component.vector = Sparsepc::Vector<double>(tmp);
            in.readRawData(reinterpret_cast<char *>(component.vector.data()),
                           tmp * sizeof(double));

            in >> tmp;
            if (tmp != 0)
            {
                component.q = Sparsepc::Vector<double>(tmp);
                in.readRawData(reinterpret_cast<char *>(component.q.data()),
                               tmp * sizeof(double));
            }
        }

        return in;
    }

} // namespace

namespace Sparsely
{
    ModelHandler::ModelHandler(LabModel &modelToBuild) : m_Model{modelToBuild}
    {
    }

    bool ModelHandler::init(const QString &fileName, bool newProject)
    {
        qDebug() << "Initializing model handler...";

        if (newProject)
        {
            auto &n = m_Model.get().m_N;
            auto &featureMatrix = m_Model.get().m_FeatureMatrix;
            auto &trace = m_Model.get().m_Trace;
            auto &standardPCs = m_Model.get().m_StandardPCs;
            auto &sparsePCs = m_Model.get().m_SparsePCs;
            auto &validatedComponents = m_Model.get().m_ValidatedComponents;
            auto &header = m_Model.get().m_Header;

            const Sparsepc::Matrix<double> rawFeatureMatrix =
                openData<double>(fileName.toStdString(), header);

            if (rawFeatureMatrix.rows() <= 1)
            {
                qCritical() << "Load data as matrix failed:" << fileName;
                return false;
            }

            featureMatrix = Sparsepc::standardScale(rawFeatureMatrix);
            n = featureMatrix.cols();
            trace = featureMatrix.colwise().squaredNorm().sum();

            standardPCs.clear();
            standardPCs.reserve(n);
            sparsePCs.clear();
            sparsePCs.reserve(n);
            validatedComponents.clear();
            validatedComponents.reserve(n);
        }
        else
        {
            if (!loadProject(fileName))
            {
                qDebug() << "...model handler not initialized";
                return false;
            }
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

        using Index = Sparsepc::Index;

        const auto &featureMatrix = m_Model.get().m_FeatureMatrix;
        const auto m = featureMatrix.rows();
        const auto n = featureMatrix.cols();
        const auto &standardPCs = m_Model.get().m_StandardPCs;
        const auto &sparsePCs = m_Model.get().m_SparsePCs;
        const auto &header = m_Model.get().m_Header;

        // Serialization
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);

        out << static_cast<qint32>(magicNumber);
        out << static_cast<qint32>(versionNumber);
        out << static_cast<qint32>(m);
        out << static_cast<qint32>(n);
        out.writeRawData(reinterpret_cast<const char *>(featureMatrix.data()),
                         m * n * sizeof(double));
        out << header;
        const Index numberOfStandardPCs = standardPCs.size();
        out << static_cast<qint32>(numberOfStandardPCs);
        for (Index j = 0; j < numberOfStandardPCs; ++j)
        {
            out << standardPCs[j];
        }
        const Index numberOfSparsePCs = sparsePCs.size();
        out << static_cast<qint32>(numberOfSparsePCs);
        for (Index j = 0; j < numberOfSparsePCs; ++j)
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

        using Index = Sparsepc::Index;

        auto &n = m_Model.get().m_N;
        auto &trace = m_Model.get().m_Trace;
        auto &standardPCs = m_Model.get().m_StandardPCs;
        auto &sparsePCs = m_Model.get().m_SparsePCs;
        auto &header = m_Model.get().m_Header;

        // Deserialization
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);
        qint32 tmp = 0;
        in >> tmp;
        const qint32 magic = tmp;
        if (magic != magicNumber)
        {
            qCritical() << "File version to old. Use old Sparsely version "
                           "0.1.0 or 0.2.0 or 0.3.0:"
                        << fileName;
            return false;
        }

        in >> tmp;
        [[maybe_unused]] const qint32 version = tmp;

        in >> tmp;
        const Index m = tmp;

        in >> tmp;
        n = tmp;

        auto &featureMatrix = m_Model.get().m_FeatureMatrix;
        featureMatrix.resize(m, n);
        in.readRawData(reinterpret_cast<char *>(featureMatrix.data()),
                       m * n * sizeof(double));
        trace = featureMatrix.colwise().squaredNorm().sum();
        qDebug() << "featureMatrix read ";

        in >> header;
        standardPCs.clear();
        standardPCs.reserve(n);
        sparsePCs.clear();
        sparsePCs.reserve(n);

        in >> tmp;
        const Index numberOfStandardPCs = tmp;
        for (Index j = 0; j < numberOfStandardPCs; ++j)
        {
            Nominees Nominees;
            in >> Nominees;
            standardPCs.push_back(std::move(Nominees));
        }
        in >> tmp;
        const Index numberOfSparsePCs = tmp;
        for (Index j = 0; j < numberOfSparsePCs; ++j)
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
        qDebug() << "Loading addon ...";
        if (!dynamicLibSolverLoaders.empty())
        {
            qDebug() << "Addon already loaded";
            return;
        }

        const auto pluginList = getPluginList(path);
        if (!pluginList.empty())
        {
            auto &dynamicLibSolverNames = m_Model.get().m_DynamicLibSolverNames;

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
} // namespace Sparsely
