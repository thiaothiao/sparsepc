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

namespace sparsely
{
    LabController::LabController(LabModel& labModel, LabWidget& labWidget, QObject* parent)
        : QObject(parent), m_LabModel{labModel}, m_LabWidget{labWidget}
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

        m_N = m_LabModel.m_N;

        m_LabWidget.setPreferences(m_Preferences);

        m_LabWidget.m_N = m_N;

        m_LabWidget.createWidget(m_LabModel.getAddonName());

        drawStandardPCs();

        if(!m_LabModel.m_SparsePCs.empty())
        {
            drawSparsePCs();
            m_LabWidget.reInitSlider(m_LabModel.m_SparsePCs.back().iCandidate);
        }

        connectWidget();

        m_LabWidget.zoomToFit();
    }

    void LabController::drawStandardPCs()
    {
        auto cummulativeVarianceStandardPCs = static_cast<double>(0);

        m_LabWidget.m_StandardPCGraphs.clear();
        m_LabWidget.m_StandardPCGraphs.reserve(m_LabModel.m_StandardPCs.size());

        for (int j=0; j< m_LabModel.m_StandardPCs.size(); ++j)
        {
            auto& standardPC = m_LabModel.m_StandardPCs[j];

            auto& component = standardPC.candidates.at(standardPC.iCandidate);

            cummulativeVarianceStandardPCs += component.value;

            QString cumulativeVarianceString = QString::number(cummulativeVarianceStandardPCs, 'f', 2);
            QString curveName = QString::number(j);

            m_LabWidget.drawStandardPC(component, cumulativeVarianceString, curveName);
        }
    }

    void LabController::drawSparsePCs()
    {
        m_LabModel.m_ValidatedComponents.clear();

        m_LabWidget.m_SparsePCGraphs.clear();
        m_LabWidget.m_SparsePCGraphs.reserve(m_LabModel.m_SparsePCs.size());
        for (int j=0; j< m_LabModel.m_SparsePCs.size(); ++j)
        {
            auto& sparsePC = m_LabModel.m_SparsePCs[j];

            auto& component = sparsePC.candidates.at(sparsePC.iCandidate);

            QString cumulativeVarianceString =
                 QString::number(m_LabModel.computeValidatedCumulativeVariance() + component.value, 'f', 2);
             QString curveName = QString::number(m_LabModel.getCurrentRank());

            if(component.state == sparsepc::ComponentState::Validated)
            {
                m_LabModel.m_ValidatedComponents.push_back(component);
            }

            m_LabWidget.drawSparsePC(component, cumulativeVarianceString, curveName);
        }
    }

    void LabController::onAddNewSparseComponent()
    {
        if(!m_LabModel.m_SparsePCs.empty())
        {// validate current selected sparse component
            auto& candidates = m_LabModel.m_SparsePCs.back().candidates;
            const auto iCandidate = m_LabModel.m_SparsePCs.back().iCandidate;

            m_LabModel.m_ValidatedComponents.push_back(candidates.at(iCandidate));
            m_LabModel.m_ValidatedComponents.back().get().state = sparsepc::ComponentState::Validated;
        }

        const auto newSparsePCColor = m_LabWidget.m_Colors[m_LabModel.m_SparsePCs.size()];

        m_LabWidget.m_ProgressBarGroupBox->setStyleSheet("QGroupBox::title { color: "+ newSparsePCColor + "; }");

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

        QString cumulativeVarianceString
            = QString::number(m_LabModel.computeValidatedCumulativeVariance() + component.value, 'f', 2);
        QString curveName = QString::number(m_LabModel.getCurrentRank());

        m_LabWidget.m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ newSparsePCColor + "; }");

        m_LabWidget.m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_LabWidget.m_SliderGroupBox);

        m_LabWidget.drawSparsePC(component, cumulativeVarianceString, curveName);
    }

    void LabController::onRemoveLastSparseComponentButton()
    {
        if(!m_LabModel.removeLastSparsePC())
        {
            return;
        }

        m_LabWidget.removeLastSparsePCDraw(m_LabModel.getICandidate());
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

        QString cumulativeVarianceString =
            QString::number(m_LabModel.computeValidatedCumulativeVariance() + component.value, 'f', 2);
        QString curveName = QString::number(m_LabModel.getCurrentRank());

        m_LabWidget.updateDraw(component, cumulativeVarianceString, curveName);
    }

    void LabController::onSelectionChanged(int index)
    {// must replot all plots
        m_LabWidget.clearAlls();

        drawStandardPCs();
        drawSparsePCs();
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
