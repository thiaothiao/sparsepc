#include "labcontroller.h"

#include <thread>
#include <functional>

#include <QFile>
#include <QString>
#include <QDir>
#include <QDebug>

namespace
{
    auto generateGraphName(int componentRank, double cumulativeVariancePercentage)
    {
        return  QString::number(
                   componentRank) + ": " + QString::number(cumulativeVariancePercentage, 'f', 1);
    }

    class DeferredUpdateForPlots final
    {// RAII
    public:
        DeferredUpdateForPlots(sparsely::LabWidget& aLabWidget)
            :labWidget{aLabWidget}
        {
            labWidget.get().setPlotUpdateEnabled(false);
        }

        ~DeferredUpdateForPlots()
        {
            labWidget.get().setPlotUpdateEnabled(true);
            labWidget.get().redrawPlot();
        }

        DeferredUpdateForPlots(const DeferredUpdateForPlots&) = delete;
        DeferredUpdateForPlots& operator=(const DeferredUpdateForPlots&) = delete;

        DeferredUpdateForPlots(DeferredUpdateForPlots&&) = delete;
        DeferredUpdateForPlots& operator=(DeferredUpdateForPlots&&) = delete;
    private:
        const std::reference_wrapper<sparsely::LabWidget> labWidget;
    };
}

namespace sparsely
{
    LabController::LabController(LabModel& labModel, LabWidget& labWidget, QObject* parent)
    : QObject(parent), m_LabModel{labModel}, m_LabWidget{labWidget}, m_N{0}
    {
        qDebug() << "Loading prefernces ...";
        m_Preferences = Preferences::load(
            QDir(QDir::currentPath() + "/configurations").absoluteFilePath("sparsely.json"));
        qDebug() << "... preferences done.";
    }

    void LabController::connectWidget()
    {
        auto& labWidget = m_LabWidget.get();
        QObject::connect(labWidget.m_PlotTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &LabController::onSelectionChanged);
        QObject::connect(labWidget.m_Slider, qOverload<int>(&QSlider::valueChanged),
            this, &LabController::updatePlot);
        QObject::connect(labWidget.m_AddNewSparseComponentButton, &QPushButton::clicked,
            this, &LabController::onAddNewSparseComponent);
        QObject::connect(labWidget.m_RemoveLastSparseComponentButton, &QPushButton::clicked,
            this, &LabController::onRemoveLastSparseComponentButton);
    }

    void LabController::init(const QString& fileName, bool newProject)
    {
        auto& labWidget = m_LabWidget.get();
        auto& labModel = m_LabModel.get();
        if(!labModel.init(fileName, newProject))
        {
            return;
        }
        m_N = labModel.getN();
        labWidget.setN( m_N);
        labWidget.createWidget(m_Preferences, labModel.getAddonName());
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        addStandardPCGraphs();
        addSparsePCGraphs();
        labWidget.reInitSlider(labModel.getiWinner());
        connectWidget();
        labWidget.zoomToFit();
    }

    void LabController::addStandardPCGraphs()
    {
        auto& labWidget = m_LabWidget.get();
        auto& labModel = m_LabModel.get();
        auto cummulativeVarianceStandardPCs = static_cast<double>(0);
        for (int j=0; j< labModel.m_StandardPCs.size(); ++j)
        {
            auto& standardPC = labModel.m_StandardPCs[j];
            auto& component = standardPC.candidates.at(standardPC.iWinner);
            cummulativeVarianceStandardPCs += component.value;
            const auto cumulativeVariancePercentage =
                labModel.computeVarianceRatio(cummulativeVarianceStandardPCs) * 100.0;
            labWidget.addStandardPCGraph(m_Preferences, component,
                generateGraphName(j, cumulativeVariancePercentage));
        }
    }

    void LabController::addSparsePCGraphs()
    {
        auto& labWidget = m_LabWidget.get();
        auto& labModel = m_LabModel.get();
        labModel.m_ValidatedComponents.clear();
        for (int j=0; j< labModel.m_SparsePCs.size(); ++j)
        {
            auto& sparsePC = labModel.m_SparsePCs[j];
            auto& component = sparsePC.candidates.at(sparsePC.iWinner);
            const auto cumulativeVariancePercentage =
                labModel.computeVarianceRatio(labModel.computeValidatedCumulativeVariance() + component.value)
                * 100.0;
            const auto componentRank = labModel.getCurrentRank();
            if(component.state == sparsepc::ComponentState::Validated)
            {
                labModel.m_ValidatedComponents.push_back(component);
            }
            labWidget.addSparsePCGraph(m_Preferences, component,
                generateGraphName(componentRank, cumulativeVariancePercentage));
        }
    }

    void LabController::onAddNewSparseComponent()
    {
        auto& labWidget = m_LabWidget.get();
        auto& labModel = m_LabModel.get();
        labModel.validateCurrentSparsePC();
        const auto newSparsePCColor = labWidget.m_Colors[labModel.m_SparsePCs.size()];
        labWidget.setProgressBarColor(newSparsePCColor);
        labWidget.m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(labWidget.m_ProgressBarGroupBox);
        labWidget.m_ProgressBar->setValue(0);
        const auto method = labWidget.m_MethodComboBox->currentData().value<Enums::Method>();
        auto& sparsePC = labModel.computeSparsePC(method, labWidget.m_ProgressBar);
        sparsePC.iWinner = labWidget.m_Slider->value();
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2000ms);
        }
        labWidget.m_ProgressBar->setValue(m_N);
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1000ms);
        }
        const auto& component = sparsePC.candidates.at(sparsePC.iWinner);
        const auto cumulativeVariancePercentage =
            labModel.computeVarianceRatio(labModel.computeValidatedCumulativeVariance() + component.value)
            * 100.0;
        const auto componentRank = labModel.getCurrentRank();
        labWidget.setSliderColor(newSparsePCColor);
        labWidget.m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(labWidget.m_SliderGroupBox);
        labWidget.addSparsePCGraph(m_Preferences, component,
            generateGraphName(componentRank, cumulativeVariancePercentage));
    }

    void LabController::onRemoveLastSparseComponentButton()
    {
        auto& labWidget = m_LabWidget.get();
        auto& labModel = m_LabModel.get();
        if(!labModel.removeLastSparsePC())
        {
            return;
        }

        labWidget.removeLastSparsePCGraph(labModel.getiWinner());
    }

    void LabController::updatePlot(int value)
    {
        auto& labWidget = m_LabWidget.get();
        auto& labModel = m_LabModel.get();
        if (labModel.m_SparsePCs.empty())
        {
            return;
        }

        auto& sparsePC = labModel.m_SparsePCs.back();
        sparsePC.iWinner = value;
        const auto& component = sparsePC.candidates.at(sparsePC.iWinner);
        const auto cumulativeVariancePercentage =
            labModel.computeVarianceRatio(labModel.computeValidatedCumulativeVariance() + component.value)
            * 100.0;
        const auto componentRank = labModel.getCurrentRank();
        labWidget.updateLastSparsePCGraph(component,
            generateGraphName(componentRank, cumulativeVariancePercentage));
    }

    void LabController::onSelectionChanged(int index)
    {
        auto& labWidget = m_LabWidget.get();
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        labWidget.clearAlls();
        addStandardPCGraphs();
        addSparsePCGraphs();
    }

    void LabController::saveProject(const QString& fileName) const
    {
        m_LabModel.get().saveProject(fileName);
    }

    void LabController::loadProject(const QString& fileName)
    {
        m_LabModel.get().loadProject(fileName);
    }

    bool LabController::projectIsEmpty() const
    {
        return m_N == 0;
    }
}
