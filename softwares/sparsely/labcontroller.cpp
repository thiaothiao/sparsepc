#include "labcontroller.h"

#include <thread>

#include <QPushButton>
#include <QSlider>
#include <QProgressBar>
#include <QFile>
#include <QColor>
#include <QString>
#include <QDir>
#include <QDebug>

namespace
{
    auto toString(double value)
    {
        return QString::number(value, 'f', 1);
    }
}

namespace sparsely
{
    LabController::LabController(LabModel& labModel, LabWidget& labWidget, QObject* parent)
    : QObject(parent), m_LabModel{labModel}, m_LabWidget{labWidget}, m_N{0}
    {
        // TODO preferences as a singleton
        qDebug() << "Loading prefernces ...";
        m_Preferences = Preferences::load(
            QDir(QDir::currentPath() + "/configurations").absoluteFilePath("sparsely.json"));
        qDebug() << "... preferences done.";
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
        if(!m_LabModel.init(fileName, newProject))
        {
            // should popup a project already loaded.
            return;
        }

        m_N = m_LabModel.getN();
        m_LabWidget.setN( m_N);

        m_LabWidget.setPreferences(m_Preferences);

        m_LabWidget.createWidget(m_LabModel.getAddonName());

        addStandardPCGraphs();
        addSparsePCGraphs();

        m_LabWidget.reInitSlider(m_LabModel.getICandidate());

        connectWidget();

        m_LabWidget.zoomToFit();
    }

    void LabController::addStandardPCGraphs()
    {
        auto cummulativeVarianceStandardPCs = static_cast<double>(0);

        for (int j=0; j< m_LabModel.m_StandardPCs.size(); ++j)
        {
            auto& standardPC = m_LabModel.m_StandardPCs[j];

            auto& component = standardPC.candidates.at(standardPC.iCandidate);

            cummulativeVarianceStandardPCs += component.value;
            const auto cumulativeVariancePercentage =
                m_LabModel.computeVarianceRatio(cummulativeVarianceStandardPCs) * 100.0;
            QString cumulativeVariancePercentageString = toString(cumulativeVariancePercentage);
            QString curveName = QString::number(j);

            m_LabWidget.addStandardPCGraph(component, cumulativeVariancePercentageString, curveName);
        }
    }

    void LabController::addSparsePCGraphs()
    {
        m_LabModel.m_ValidatedComponents.clear();

        for (int j=0; j< m_LabModel.m_SparsePCs.size(); ++j)
        {
            auto& sparsePC = m_LabModel.m_SparsePCs[j];

            auto& component = sparsePC.candidates.at(sparsePC.iCandidate);

            const auto cumulativeVariancePercentage =
                m_LabModel.computeVarianceRatio(m_LabModel.computeValidatedCumulativeVariance() + component.value)
                * 100.0;
            QString cumulativeVariancePercentageString = toString(cumulativeVariancePercentage);
             QString curveName = QString::number(m_LabModel.getCurrentRank());

            if(component.state == sparsepc::ComponentState::Validated)
            {
                m_LabModel.m_ValidatedComponents.push_back(component);
            }

            m_LabWidget.addSparsePCGraph(component, cumulativeVariancePercentageString, curveName);
        }
    }

    void LabController::onAddNewSparseComponent()
    {
        m_LabModel.validateCurrentSparsePC();

        const auto newSparsePCColor = m_LabWidget.m_Colors[m_LabModel.m_SparsePCs.size()];

        m_LabWidget.setProgressBarColor(newSparsePCColor);

        m_LabWidget.m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_LabWidget.m_ProgressBarGroupBox);

        m_LabWidget.m_ProgressBar->setValue(0);

        const auto method = m_LabWidget.m_MethodComboBox->currentData().value<Enums::Method>();

        auto& sparsePC = m_LabModel.computeSparsePC(method, m_LabWidget.m_ProgressBar);

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

        const auto cumulativeVariancePercentage =
            m_LabModel.computeVarianceRatio(m_LabModel.computeValidatedCumulativeVariance() + component.value)
            * 100.0;

        QString cumulativeVariancePercentageString = toString(cumulativeVariancePercentage);

        QString curveName = QString::number(m_LabModel.getCurrentRank());
        m_LabWidget.setSliderColor(newSparsePCColor);

        m_LabWidget.m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_LabWidget.m_SliderGroupBox);

        m_LabWidget.addSparsePCGraph(component, cumulativeVariancePercentageString, curveName);
    }

    void LabController::onRemoveLastSparseComponentButton()
    {
        if(!m_LabModel.removeLastSparsePC())
        {
            return;
        }

        m_LabWidget.removeLastSparsePCGraph(m_LabModel.getICandidate());
    }

    void LabController::updatePlot(int value)
    {
        if (m_LabModel.m_SparsePCs.empty())
        {
            return;
        }

        auto& sparsePC = m_LabModel.m_SparsePCs.back();

        sparsePC.iCandidate = value;

        const auto& component = sparsePC.candidates.at(sparsePC.iCandidate);

        const auto cumulativeVariancePercentage =
            m_LabModel.computeVarianceRatio(m_LabModel.computeValidatedCumulativeVariance() + component.value)
            * 100.0;

        QString cumulativeVariancePercentageString = toString(cumulativeVariancePercentage);

        QString curveName = QString::number(m_LabModel.getCurrentRank());

        m_LabWidget.updateLastSparsePCGraph(component, cumulativeVariancePercentageString, curveName);
    }

    void LabController::onSelectionChanged(int index)
    {
        m_LabWidget.clearAlls();
        addStandardPCGraphs();
        addSparsePCGraphs();
    }

    void LabController::saveProject(const QString& fileName)
    {
        m_LabModel.saveProject(fileName);
    }

    void LabController::loadProject(const QString& fileName)
    {
        m_LabModel.loadProject(fileName);
    }
}
