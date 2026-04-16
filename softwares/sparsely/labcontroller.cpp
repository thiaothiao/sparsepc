#include "labcontroller.h"

#include <functional>
#include <thread>

#include <QDir>
#include <QFile>
#include <QProgressDialog>
#include <QString>

#include "labprogressdialog.h"

namespace
{
    auto generateGraphName(int componentRank,
                           double cumulativeVariancePercentage)
    {
        return QString::number(componentRank) + ": " +
               QString::number(cumulativeVariancePercentage, 'f', 1);
    }

    class DeferredUpdateForPlots final
    { // RAII
      public:
        DeferredUpdateForPlots(sparsely::LabWidget &aLabWidget)
            : labWidget{aLabWidget}
        {
            labWidget.get().setPlotUpdateEnabled(false);
        }

        ~DeferredUpdateForPlots()
        {
            labWidget.get().setPlotUpdateEnabled(true);
            labWidget.get().redrawPlot();
        }

        DeferredUpdateForPlots(const DeferredUpdateForPlots &) = delete;
        DeferredUpdateForPlots &
        operator=(const DeferredUpdateForPlots &) = delete;

        DeferredUpdateForPlots(DeferredUpdateForPlots &&) = delete;
        DeferredUpdateForPlots &operator=(DeferredUpdateForPlots &&) = delete;

      private:
        const std::reference_wrapper<sparsely::LabWidget> labWidget;
    };

    class SliderDisconnectConnect final
    { // RAII
      public:
        SliderDisconnectConnect(QSlider &aSlider,
                                sparsely::LabController &aController)
            : qSlider{aSlider}, labController{aController}
        {
            QObject::disconnect(
                &qSlider.get(), qOverload<int>(&QSlider::valueChanged),
                &labController.get(), &sparsely::LabController::updatePlot);
        }

        ~SliderDisconnectConnect()
        {
            QObject::connect(
                &qSlider.get(), qOverload<int>(&QSlider::valueChanged),
                &labController.get(), &sparsely::LabController::updatePlot);
        }

        SliderDisconnectConnect(const SliderDisconnectConnect &) = delete;
        SliderDisconnectConnect &
        operator=(const SliderDisconnectConnect &) = delete;

        SliderDisconnectConnect(SliderDisconnectConnect &&) = delete;
        SliderDisconnectConnect &operator=(SliderDisconnectConnect &&) = delete;

      private:
        const std::reference_wrapper<QSlider> qSlider;
        const std::reference_wrapper<sparsely::LabController> labController;
    };

    class NoEscapeNoXCloseQProgressDialog : public QProgressDialog
    {
      public:
        NoEscapeNoXCloseQProgressDialog(const QString &labelText,
                                        const QString &cancelButtonText,
                                        int minimum, int maximum,
                                        QWidget *parent = nullptr)
            : QProgressDialog(labelText, cancelButtonText, minimum, maximum,
                              parent,
                              Qt::WindowFlags() & ~Qt::WindowCloseButtonHint)
        {
        }

      protected:
        bool event(QEvent *event) override
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent && keyEvent->key() == Qt::Key_Escape)
            {
                keyEvent->accept();
                return true;
            }
            return QProgressDialog::event(event);
        }
    };
} // namespace

namespace sparsely
{
    LabController::LabController(LabModel &labModel, LabWidget &labWidget,
                                 QObject *parent)
        : QObject(parent), m_LabModel{labModel}, m_LabWidget{labWidget}, m_N{0}
    {
        QDir dir = QDir::current();
        dir.cdUp();
        m_Preferences =
            Preferences::load(QDir(dir.path() + "/configurations")
                                  .absoluteFilePath("sparsely.json"));
    }

    void LabController::connectWidget()
    {
        auto &labWidget = m_LabWidget.get();
        QObject::connect(labWidget.m_PlotTypeComboBox,
                         qOverload<int>(&QComboBox::currentIndexChanged), this,
                         &LabController::onSelectionChanged);
        QObject::connect(labWidget.m_Slider,
                         qOverload<int>(&QSlider::valueChanged), this,
                         &LabController::updatePlot);
        QObject::connect(labWidget.m_AddNewSparseComponentButton,
                         &QPushButton::clicked, this,
                         &LabController::onAddNewSparseComponent);
        QObject::connect(labWidget.m_RemoveLastSparseComponentButton,
                         &QPushButton::clicked, this,
                         &LabController::onRemoveLastSparseComponentButton);
    }

    void LabController::welcome()
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        labWidget.welcome(labModel.getAddonName());
    }

    void LabController::init(const QString &fileName, bool newProject)
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        if (!labModel.init(fileName, newProject))
        {
            return;
        }
        m_N = labModel.getN();
        labWidget.setN(m_N);
        labWidget.updateWidget(m_Preferences, labModel.getAddonName());
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        addStandardPCGraphs();
        addSparsePCGraphs();
        labWidget.reInitSlider(labModel.getiWinner());
        connectWidget();
        labWidget.zoomToFit();
    }

    void LabController::addStandardPCGraphs()
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        auto cummulativeVarianceStandardPCs = static_cast<double>(0);
        for (int j = 0; j < labModel.m_StandardPCs.size(); ++j)
        {
            auto &standardPC = labModel.m_StandardPCs[j];
            auto &component = standardPC.candidates.at(standardPC.iWinner);
            cummulativeVarianceStandardPCs += component.value;
            const auto cumulativeVariancePercentage =
                labModel.computeVarianceRatio(cummulativeVarianceStandardPCs) *
                100.0;
            labWidget.addStandardPCGraph(
                m_Preferences, component,
                generateGraphName(j, cumulativeVariancePercentage));
        }
    }

    void LabController::addSparsePCGraphs()
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        labModel.m_ValidatedComponents.clear();
        for (int j = 0; j < labModel.m_SparsePCs.size(); ++j)
        {
            auto &sparsePC = labModel.m_SparsePCs[j];
            auto &component = sparsePC.candidates.at(sparsePC.iWinner);
            const auto cumulativeVariancePercentage =
                labModel.computeVarianceRatio(
                    labModel.computeValidatedCumulativeVariance() +
                    component.value) *
                100.0;
            const auto componentRank = labModel.getCurrentRank();
            if (component.state == sparsepc::ComponentState::Validated)
            {
                labModel.m_ValidatedComponents.push_back(component);
            }
            labWidget.addSparsePCGraph(
                m_Preferences, component,
                generateGraphName(componentRank, cumulativeVariancePercentage));
        }
    }

    void LabController::onAddNewSparseComponent()
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();

        const SliderDisconnectConnect sliderDisconnectConnect(
            *labWidget.m_Slider, *this);

        labModel.validateCurrentSparsePC();

        const auto newSparsePCColor =
            labWidget.m_Colors[labModel.m_SparsePCs.size()];

        NoEscapeNoXCloseQProgressDialog qProgressDialog(
            "Computing sparse pcs...", "Abort", 0, m_N);
        ProgressDialog progressDialog(qProgressDialog);

        qProgressDialog.setStyleSheet(
            "QProgressBar::chunk { background-color:" + newSparsePCColor +
            "; }");

        qProgressDialog.setWindowModality(Qt::WindowModal);
        qProgressDialog.setMinimumDuration(0);

        const auto method =
            labWidget.m_MethodComboBox->currentData().value<Enums::Method>();
        auto &sparsePC = labModel.computeSparsePC(method, &progressDialog);

        if (progressDialog.wasCanceled())
        {
            labModel.removeLastSparsePC();
            return;
        }

        sparsePC.iWinner = labWidget.m_Slider->value();

        const auto &component = sparsePC.candidates.at(sparsePC.iWinner);
        const auto cumulativeVariancePercentage =
            labModel.computeVarianceRatio(
                labModel.computeValidatedCumulativeVariance() +
                component.value) *
            100.0;
        const auto componentRank = labModel.getCurrentRank();
        labWidget.setSliderColor(newSparsePCColor);
        labWidget.addSparsePCGraph(
            m_Preferences, component,
            generateGraphName(componentRank, cumulativeVariancePercentage));
    }

    void LabController::onRemoveLastSparseComponentButton()
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        if (!labModel.removeLastSparsePC())
        {
            return;
        }

        labWidget.removeLastSparsePCGraph(labModel.getiWinner());
    }

    void LabController::updatePlot(int value)
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        if (labModel.m_SparsePCs.empty())
        {
            return;
        }

        auto &sparsePC = labModel.m_SparsePCs.back();
        sparsePC.iWinner = value;
        const auto &component = sparsePC.candidates.at(sparsePC.iWinner);
        const auto cumulativeVariancePercentage =
            labModel.computeVarianceRatio(
                labModel.computeValidatedCumulativeVariance() +
                component.value) *
            100.0;
        const auto componentRank = labModel.getCurrentRank();
        labWidget.updateLastSparsePCGraph(
            component,
            generateGraphName(componentRank, cumulativeVariancePercentage));
    }

    void LabController::onSelectionChanged(int index)
    {
        auto &labWidget = m_LabWidget.get();
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        labWidget.clearAlls();
        addStandardPCGraphs();
        addSparsePCGraphs();
    }

    void LabController::saveProject(const QString &fileName) const
    {
        m_LabModel.get().saveProject(fileName);
    }

    void LabController::loadProject(const QString &fileName)
    {
        m_LabModel.get().loadProject(fileName);
    }
} // namespace sparsely
