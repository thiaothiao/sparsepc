#include "labcontroller.h"

#include <thread>
#include <utility>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QProgressBar>
#include <QPen>
#include <QFile>
#include <QColor>
#include <QString>
#include <QDataStream>
#include <QDir>
#include <QDebug>

#include "jkqtplotter/graphs/jkqtpfilledcurve.h"
#include "jkqtplotter/graphs/jkqtpimpulses.h"

#include "PreferencesDialog.h"

namespace
{
    using BackwardGSPA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::BackwardGspcaModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;

    using ForwardGSPCA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::ForwardGspcaModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;

    using DCA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::DcaModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;

    using CustomSolver = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::CustomSolverModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;

    using DynamicLibSolver = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::DynamicLibSolverModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;

    auto getPluginList()
    {
        QDir path(QDir::currentPath() + "/addons");

        qDebug() << "current path is: " << path.currentPath();

        QStringList filters;
        filters << "*.dll" << "*.so" << "*.dylib";

        return path.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    }
}

namespace sparsely
{
    LabController::LabController(LabWidget& labWidget, QObject* parent)
        : QObject(parent), m_LabWidget{labWidget}, m_Sigma{}, m_ValidatedComponents{},
        m_SparsePCs{}, m_StandardPCs{}, m_CummulativeVarianceStandardPCs{0.0},
        m_CummulativeVarianceSparsePCs{0}
    {
        // TODO preferences as a singleton
        qDebug() << "Loading prefernces ...";
        m_Preferences = Preferences::load(
            QDir(QDir::currentPath() + "/configurations").absoluteFilePath("sparsely.json"));
        qDebug() << "... preferences done.";
        qDebug() << "Loading addons ...";

        const auto pluginList = getPluginList();
        if(!pluginList.empty())
        {
            m_DynamicLibSolverLoaders.reserve(pluginList.size());
            m_DynamicLibSolverNames.reserve(pluginList.size());

            for (const auto &fileInfo : pluginList)
            {
                QString noExtensionAbsolutePath = QDir(fileInfo.absolutePath()).filePath(fileInfo.baseName());
                qDebug() << "Loading " << noExtensionAbsolutePath << "...";
                m_DynamicLibSolverLoaders.push_back(
                    std::make_unique<QLibrary>(noExtensionAbsolutePath));
                if(m_DynamicLibSolverLoaders.back()->load())
                {
                    m_DynamicLibSolverNames.push_back(fileInfo.baseName());
                    qDebug() << " ...loaded.";
                }
                else
                {
                    qDebug() << " ...loading failed.";
                    m_DynamicLibSolverLoaders.pop_back();
                }
            }

            //foreach(auto fileName, path.entryList(QDir::Files))

            qDebug() << "...Addons loaded.";
        }
    }

    void LabController::connectWidget()
    {
        QObject::connect(m_LabWidget.m_PlotTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                         this, &LabController::onSelectionChanged);
        QObject::connect(m_LabWidget.m_Slider, qOverload<int>(&QSlider::valueChanged),
                         this, &LabController::updatePlot);
        QObject::connect(m_LabWidget.m_AddNewSparseComponentButton, &QPushButton::clicked,
                         this, &LabController::onAddNewSparseComponent);
        QObject::connect(m_LabWidget.m_RemoveLastSparseComponentButton, &QPushButton::clicked,
                         this, &LabController::onRemoveLastSparseComponentButton);
    }

    void LabController::init(const QString& fileName, bool newProject)
    {
        if(m_Sigma.size() != 0)
        {
            // should popup a project already loaded.
            return;
        }

        if(newProject)
        {
            m_Sigma = sparsepc::openData<double>(fileName.toStdString(), ';');

            m_N = m_Sigma.cols();

            m_ValidatedComponents.clear();
            m_ValidatedComponents.reserve(m_N);
            m_SparsePCs.clear();
            m_SparsePCs.reserve(m_N);

            m_LabWidget.m_N = m_N;

            m_LabWidget.createWidget();

            computeStandardPCs();
            drawStandardPCs();
        }
        else
        {
            loadProject(fileName);

            m_LabWidget.m_N = m_N;

            m_LabWidget.createWidget();

            drawStandardPCs();
            drawSparsePCs();
        }

        if(!m_SparsePCs.empty())
        {
            m_LabWidget.reInitSlider(m_SparsePCs.back().iCandidate);
        }

        connectWidget();

        m_LabWidget.zoomToFit();
    }

    void LabController::computeStandardPCs()
    {
        using Index = sparsepc::Index;
        const DCA::Param param{{static_cast<Index>(m_N),
                                static_cast<Index>(m_N), static_cast<Index>(m_N)} };
        auto components = DCA{ param }.run(m_Sigma);
        m_StandardPCs.clear();
        m_StandardPCs.reserve(components.size());
        for (int j=0; j < components.size(); ++j)
        {
            m_StandardPCs.emplace_back();
            auto& standardPC = m_StandardPCs.back();
            standardPC.iCandidate = m_N;
            auto& component = components[j];
            component.state = sparsepc::ComponentState::Validated;
            standardPC.candidates.emplace(m_N, std::move(components[j]));
        }
    }

    void LabController::drawStandardPCs()
    {
        m_CummulativeVarianceStandardPCs = static_cast<double>(0);

        m_LabWidget.m_StandardPCGraphs.clear();
        m_LabWidget.m_StandardPCGraphs.reserve(m_StandardPCs.size());

        for (int j=0; j< m_StandardPCs.size(); ++j)
        {
            auto& standardPC = m_StandardPCs[j];

            auto& component = standardPC.candidates.at(standardPC.iCandidate);

            m_CummulativeVarianceStandardPCs += component.value;

            QString cumulativeVarianceString = QString::number(m_CummulativeVarianceStandardPCs, 'f', 2);
            QString curveName = QString::number(j);

            m_LabWidget.drawStandardPC(component, cumulativeVarianceString, curveName);
        }
    }

    void LabController::drawSparsePCs()
    {
        m_CummulativeVarianceSparsePCs = static_cast<double>(0);

        m_LabWidget.m_SparsePCGraphs.clear();
        m_LabWidget.m_SparsePCGraphs.reserve(m_SparsePCs.size());
        for (int j=0; j< m_SparsePCs.size(); ++j)
        {
            auto& sparsePC = m_SparsePCs[j];

            auto& component = sparsePC.candidates.at(sparsePC.iCandidate);

            const auto cummulativeVariance = m_CummulativeVarianceSparsePCs + component.value;

            if(component.state == sparsepc::ComponentState::Validated)
            {
                m_ValidatedComponents.push_back(component);
                m_CummulativeVarianceSparsePCs = cummulativeVariance;
            }

            QString cumulativeVarianceString = QString::number(cummulativeVariance, 'f', 2);
            QString curveName = QString::number(j);

            m_LabWidget.drawSparsePC(component, cumulativeVarianceString, curveName);
        }
    }

    void LabController::onAddNewSparseComponent()
    {
        if(!m_SparsePCs.empty())
        {// validate current selected sparse component
            auto& candidates = m_SparsePCs.back().candidates;
            const auto iCandidate = m_SparsePCs.back().iCandidate;

            m_ValidatedComponents.push_back(candidates.at(iCandidate));
            m_ValidatedComponents.back().get().state = sparsepc::ComponentState::Validated;
            m_CummulativeVarianceSparsePCs += m_ValidatedComponents.back().get().value;
        }


        auto& sparsePC = m_SparsePCs.emplace_back();

        sparsePC.iCandidate = -1;

        const auto newSparsePCColor = m_LabWidget.m_Colors[m_SparsePCs.size()-1];

        m_LabWidget.m_ProgressBarGroupBox->setStyleSheet("QGroupBox::title { color: "+ newSparsePCColor + "; }");

        m_LabWidget.m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_LabWidget.m_ProgressBarGroupBox);

        m_LabWidget.m_ProgressBar->setValue(0);

        const auto method = m_LabWidget.m_MethodComboBox->currentData().value<Enums::Method>();
        switch (method)
        {
        case Enums::Method::DCA:
        {
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_Sigma, DCA::ModelParam(1), m_ValidatedComponents, m_LabWidget.m_ProgressBar);
            break;
        }
        case Enums::Method::BGSPCA:
        {
            sparsePC.candidates = BackwardGSPA::computeNextComponentCandidates(
                m_Sigma, BackwardGSPA::ModelParam(1), m_ValidatedComponents, m_LabWidget.m_ProgressBar);
            break;
        }
        case Enums::Method::FGSPCA:
        {
            sparsePC.candidates = ForwardGSPCA::computeNextComponentCandidates(
                m_Sigma, ForwardGSPCA::ModelParam(1), m_ValidatedComponents, m_LabWidget.m_ProgressBar);
            break;
        }
        case Enums::Method::CUSTOM:
        {
            sparsePC.candidates = CustomSolver::computeNextComponentCandidates(
                m_Sigma, CustomSolver::ModelParam(1), m_ValidatedComponents, m_LabWidget.m_ProgressBar);
            break;
        }
        case Enums::Method::USERDYNAMICLIB:
        {
            auto& library = m_DynamicLibSolverLoaders.at(0);
            if(library)
            {
                auto computeSparseEigenVector =
                    (ComputeSparseEigenVector)library->resolve("computeSparseEigenVector");
                if (computeSparseEigenVector)
                {
                    sparsePC.candidates = DynamicLibSolver::computeNextComponentCandidates(
                        m_Sigma, DynamicLibSolver::ModelParam(computeSparseEigenVector), m_ValidatedComponents, m_LabWidget.m_ProgressBar);
                }
            }
            break;
        }
        default:
        {
            sparsePC.candidates = DCA::computeNextComponentCandidates(
                m_Sigma, DCA::ModelParam(1), m_ValidatedComponents, m_LabWidget.m_ProgressBar);
            break;
        }
        }

        for(auto& [k, candidate]: sparsePC.candidates)
        {
            candidate.state = sparsepc::ComponentState::Unvalidated;
        }

        sparsePC.iCandidate = m_LabWidget.m_Slider->value();

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2000ms);
        }

        m_LabWidget.m_ProgressBar->setValue(m_N);

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1000ms);
        }

        const auto& component = sparsePC.candidates.at(sparsePC.iCandidate);

        QString cumulativeVarianceString = QString::number(m_CummulativeVarianceSparsePCs + component.value, 'f', 2);
        QString curveName = QString::number(m_ValidatedComponents.size());

        m_LabWidget.m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ newSparsePCColor + "; }");

        m_LabWidget.m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_LabWidget.m_SliderGroupBox);

        m_LabWidget.drawSparsePC(component, cumulativeVarianceString, curveName);
    }

    void LabController::onRemoveLastSparseComponentButton()
    {
        if(m_SparsePCs.empty())
        {
            return;
        }

        if(!m_ValidatedComponents.empty())
        {
            m_ValidatedComponents.back().get().state = sparsepc::ComponentState::Unvalidated;
            m_CummulativeVarianceSparsePCs -= m_ValidatedComponents.back().get().value;
            m_ValidatedComponents.pop_back();
        }

        m_SparsePCs.pop_back();

        const int sliderValue = m_SparsePCs.empty()? 0 : m_SparsePCs.back().iCandidate;

        m_LabWidget.removeLastSparsePCDraw(sliderValue);
    }

    void LabController::updatePlot(int value)
    {
        if (m_SparsePCs.empty())
        {
            return;
        }

        auto& sparsePC = m_SparsePCs.back();

        sparsePC.iCandidate = value;

        const auto& component = sparsePC.candidates.at(sparsePC.iCandidate);

        QString cumulativeVarianceString = QString::number(m_CummulativeVarianceSparsePCs + component.value, 'f', 2);
        QString curveName = QString::number(m_ValidatedComponents.size());

        m_LabWidget.updateDraw(component, cumulativeVarianceString, curveName);
    }

    void LabController::onSelectionChanged(int index)
    {// must replot all plots

        m_LabWidget.clearAlls();

        drawStandardPCs();
        drawSparsePCs();
    }

    // write operator
    QDataStream &operator<<(QDataStream &out, const MyClass &user)
    {
        out << static_cast<qint32>(user.iCandidate);
        out << static_cast<qint32>(user.candidates.size());
        for(auto& [k, component]:user.candidates)
        {
            out << static_cast<qint32>(k);
            out << static_cast<qint32>(std::to_underlying(component.state));
            out << static_cast<double>(component.value);
            out << static_cast<qint32>(component.vector.size());
            out.writeRawData(reinterpret_cast<const char*>(
                                 component.vector.data()), component.vector.size() * sizeof(double));
            out << static_cast<qint32>(component.q.size());
            if(component.q.size() != 0)
            {
                out.writeRawData(reinterpret_cast<const char*>(
                                     component.q.data()), component.q.size() * sizeof(double));
            }
        }
        return out;
    }

    // read operator
    QDataStream &operator>>(QDataStream &in, MyClass &user)
    {
        in >> user.iCandidate;
        qint32 candidatesSize = -1;
        in >> candidatesSize;
        for(qint32 j = 0; j < candidatesSize; ++j)
        {
            qint32 valInt = -1;
            in >> valInt;
            auto& component = user.candidates[valInt];
            in >> valInt;
            component.state = static_cast<sparsepc::ComponentState>(valInt);
            auto val = static_cast<double>(-1);
            in >> val;
            component.value = val;
            in >> valInt;
            component.vector = sparsepc::Vector<double>(valInt);
            in.readRawData(reinterpret_cast<char*>(
                               component.vector.data()), valInt * sizeof(double));

            in >> valInt;
            if(valInt != 0)
            {
                component.q = sparsepc::Vector<double>(valInt);
                in.readRawData(reinterpret_cast<char*>(
                                   component.q.data()), valInt * sizeof(double));
            }
        }

        return in;
    }

    void LabController::saveProject(const QString& filename)
    {
        if(m_Sigma.size() == 0)
        {
            // should popup we do not save an empty project.
            return;
        }

        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
        {// Serialization
            QDataStream out(&file);
            out.setVersion(QDataStream::Qt_6_0);// version for forward/backward compatibility
            out << static_cast<qint32>(m_N);
            out.writeRawData(reinterpret_cast<const char*>(m_Sigma.data()), m_N * m_N * sizeof(double));
            const auto standardPCsSize = static_cast<qint32>(m_StandardPCs.size());
            out << standardPCsSize;
            for(qint32 j = 0; j < standardPCsSize; ++j)
            {
                out << m_StandardPCs[j] ;
            }
            const auto sparsePCsSize = static_cast<qint32>(m_SparsePCs.size());
            out << sparsePCsSize;
            for(qint32 j = 0; j < sparsePCsSize; ++j)
            {
                out << m_SparsePCs[j];
            }
            file.close();
        }
    }

    void LabController::loadProject(const QString& fileName)
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {// Deserialization
            QDataStream in(&file);
            in.setVersion(QDataStream::Qt_6_0);
            qint32 intVal = -1;
            in >> intVal;
            m_N = intVal;
            m_Sigma.resize(m_N, m_N);
            in.readRawData(reinterpret_cast<char*>(m_Sigma.data()), m_N * m_N * sizeof(double));
            m_StandardPCs.clear();
            m_StandardPCs.reserve(m_N);
            m_SparsePCs.clear();
            m_SparsePCs.reserve(m_N);
            in >> intVal;
            for(qint32 j = 0; j < intVal; ++j)
            {
                MyClass myClass;
                in >> myClass;
                m_StandardPCs.push_back(std::move(myClass));
            }
            in >> intVal;
            for(qint32 j = 0; j < intVal; ++j)
            {
                MyClass myClass;
                in >> myClass;
                m_SparsePCs.push_back(std::move(myClass));
            }
            file.close();
        }
    }
}
