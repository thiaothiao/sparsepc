#include <labcontroller.h>

#include <functional>

#include <QComboBox>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QString>
#include <QtLogging>

#include <labenums.h>
#include <labmodel.h>
#include <labprogressdialog.h>
#include <labwidget.h>

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
        explicit DeferredUpdateForPlots(Sparsely::LabWidget &aLabWidget)
            : labWidget{aLabWidget}
        {
            labWidget.get().setPlotUpdateEnabled(false);
        }

        ~DeferredUpdateForPlots()
        {
            labWidget.get().zoomToFit();
            labWidget.get().setPlotUpdateEnabled(true);
            labWidget.get().redrawPlot();
        }

        DeferredUpdateForPlots(const DeferredUpdateForPlots &) = delete;
        DeferredUpdateForPlots &
        operator=(const DeferredUpdateForPlots &) = delete;

        DeferredUpdateForPlots(DeferredUpdateForPlots &&) = delete;
        DeferredUpdateForPlots &operator=(DeferredUpdateForPlots &&) = delete;

      private:
        const std::reference_wrapper<Sparsely::LabWidget> labWidget;
    };

    class SliderDisconnectConnect final
    { // RAII
      public:
        SliderDisconnectConnect(QSlider &aSlider,
                                Sparsely::LabController &aController)
            : qSlider{aSlider}, labController{aController}
        {
            QObject::disconnect(
                &qSlider.get(), qOverload<int>(&QSlider::valueChanged),
                &labController.get(), &Sparsely::LabController::onValueChanged);
        }

        ~SliderDisconnectConnect()
        {
            QObject::connect(
                &qSlider.get(), qOverload<int>(&QSlider::valueChanged),
                &labController.get(), &Sparsely::LabController::onValueChanged);
        }

        SliderDisconnectConnect(const SliderDisconnectConnect &) = delete;
        SliderDisconnectConnect &
        operator=(const SliderDisconnectConnect &) = delete;

        SliderDisconnectConnect(SliderDisconnectConnect &&) = delete;
        SliderDisconnectConnect &operator=(SliderDisconnectConnect &&) = delete;

      private:
        const std::reference_wrapper<QSlider> qSlider;
        const std::reference_wrapper<Sparsely::LabController> labController;
    };

    class NoEscapeQProgressDialog : public QProgressDialog
    {
      public:
        NoEscapeQProgressDialog(const QString &labelText,
                                const QString &cancelButtonText, int minimum,
                                int maximum, QWidget *parent = nullptr)
            : QProgressDialog(labelText, cancelButtonText, minimum, maximum,
                              parent)
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

namespace Sparsely
{
    LabController::LabController(LabModel &labModel, LabWidget &labWidget,
                                 QObject *parent)
        : QObject(parent), m_LabModel{labModel}, m_LabWidget{labWidget},
          m_ModelHandler{labModel}, m_N{0}
    {
        m_Preferences.load();
        m_ModelHandler.loadAddon(m_Preferences.addonsPath);
    }

    void LabController::connectWidget()
    {
        auto &labWidget = m_LabWidget.get();
        QObject::connect(labWidget.m_PlotTypeComboBox,
                         qOverload<int>(&QComboBox::currentIndexChanged), this,
                         &LabController::onSelectionChanged);
        QObject::connect(labWidget.m_Slider,
                         qOverload<int>(&QSlider::valueChanged), this,
                         &LabController::onValueChanged);
        QObject::connect(labWidget.m_AddNewComponentButton,
                         &QPushButton::clicked, this,
                         &LabController::onAddNewComponent);
        QObject::connect(labWidget.m_RemoveLastComponentButton,
                         &QPushButton::clicked, this,
                         &LabController::onRemoveLastComponent);
    }

    void LabController::welcome()
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        labWidget.welcome(labModel.getAddonName());
    }

    bool LabController::init(const QString &fileName, bool newProject)
    {
        qDebug() << "Initializing controller ...";
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        if (!m_ModelHandler.init(fileName, newProject))
        {
            qCritical() << "Cannot initialize Model Handler";
            return false;
        }
        m_N = labModel.getN();
        labWidget.setN(m_N);
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        labWidget.updateWidget(m_Preferences, labModel.getAddonName(),
                               labModel.m_Header);
        addStandardPCGraphs();
        addSparsePCGraphs();
        labWidget.reInitSlider(labModel.getiWinner());
        connectWidget();
        qDebug() << "... controller initialized";
        return true;
    }

    void LabController::addStandardPCGraphs()
    {
        qDebug() << "Adding bulk standard component graphs ...";
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
        qDebug() << "... bulk standard component graphs added";
    }

    void LabController::addSparsePCGraphs()
    {
        qDebug() << "Adding bulk sparse component graphs ...";
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        labModel.m_ValidatedComponents.clear();
        for (int j = 0; j < labModel.m_SparsePCs.size(); ++j)
        {
            auto &sparsePC = labModel.m_SparsePCs[j];
            auto &component = sparsePC.candidates.at(sparsePC.iWinner);
            const auto cumulativeVariancePercentage =
                labModel.computeVarianceRatio(
                    labModel
                        .computePreviousSparseComponentRoundsCumulativeVariance() +
                    component.value) *
                100.0;
            const auto currentSparseComponentRound =
                labModel.getCurrentSparseComponentRound();
            if (component.state == Sparsepc::ComponentState::Validated)
            {
                labModel.m_ValidatedComponents.push_back(component);
            }
            labWidget.addSparsePCGraph(
                m_Preferences, component,
                generateGraphName(currentSparseComponentRound,
                                  cumulativeVariancePercentage));
        }
        qDebug() << "... bulk sparse component graphs added";
    }

    void LabController::onAddNewComponent()
    {
        qDebug() << "Adding new component ...";
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        if (labWidget.m_StandardPCRadioButton->isChecked())
        {
            if (labModel.m_StandardPCs.size() >= labWidget.m_Colors.size())
            {
                labWidget.lauchMessageBox(
                    tr("Maximum number of standard components reached. You can "
                       "change it from preferences then relaunch the "
                       "application."));
                return;
            }
            const auto newStandardPCColor =
                labWidget.m_Colors[labModel.m_StandardPCs.size()];
            NoEscapeQProgressDialog qProgressDialog(
                tr("Computing standard pcs..."), tr("Abort"), 0, m_N,
                &labWidget);
            ProgressDialog progressDialog(qProgressDialog);

            qProgressDialog.setStyleSheet(
                "QProgressBar::chunk { background-color:" + newStandardPCColor +
                "; }");
            qProgressDialog.setWindowFlags(qProgressDialog.windowFlags() &
                                           ~Qt::WindowCloseButtonHint);
            qProgressDialog.setWindowModality(Qt::WindowModal);
            qProgressDialog.setMinimumDuration(0);

            const auto &standardPC =
                labModel.computeNextRoundStandardComponent(&progressDialog);

            progressDialog.setValue(m_N);
            if (progressDialog.wasCanceled())
            {
                labModel.removeLastStandardComponent();
                qDebug() << "Progress cancelled";
                return;
            }

            const auto &component =
                standardPC.candidates.at(standardPC.iWinner);
            const auto cumulativeVariancePercentage =
                labModel.computeVarianceRatio(
                    labModel.computeStandardComponentsCumulativeVariance()) *
                100.0;
            labWidget.addStandardPCGraph(
                m_Preferences, component,
                generateGraphName(labModel.m_StandardPCs.size() - 1,
                                  cumulativeVariancePercentage));
            qDebug() << "... standard component added";
            return;
        }

        if (labModel.m_SparsePCs.size() >= labWidget.m_Colors.size())
        {
            labWidget.lauchMessageBox(tr(
                "Maximum number of sparse components reached. You can change "
                "it from preferences then relaunch the application."));
            return;
        }

        const SliderDisconnectConnect sliderDisconnectConnect(
            *labWidget.m_Slider, *this);

        labModel.validateCurrentSparseComponent();

        const auto newSparsePCColor =
            labWidget.m_Colors[labModel.m_SparsePCs.size()];

        NoEscapeQProgressDialog qProgressDialog(
            tr("Computing sparse pcs..."), tr("Abort"), 0, m_N, &labWidget);
        ProgressDialog progressDialog(qProgressDialog);

        qProgressDialog.setStyleSheet(
            "QProgressBar::chunk { background-color:" + newSparsePCColor +
            "; }");
        qProgressDialog.setWindowFlags(qProgressDialog.windowFlags() &
                                       ~Qt::WindowCloseButtonHint);
        qProgressDialog.setWindowModality(Qt::WindowModal);
        qProgressDialog.setMinimumDuration(0);

        const auto method =
            labWidget.m_MethodComboBox->currentData().value<Enums::Method>();
        auto &sparsePC = labModel.computeNextRoundSparseComponentCandidates(
            method, &progressDialog);

        if (progressDialog.wasCanceled())
        {
            labModel.removeLastSparsePC();
            qDebug() << "Progress cancelled";
            return;
        }

        sparsePC.iWinner = labWidget.m_Slider->value();

        const auto &component = sparsePC.candidates.at(sparsePC.iWinner);
        const auto cumulativeVariancePercentage =
            labModel.computeVarianceRatio(
                labModel
                    .computePreviousSparseComponentRoundsCumulativeVariance() +
                component.value) *
            100.0;
        const auto currentSparseComponentRound =
            labModel.getCurrentSparseComponentRound();
        labWidget.setSliderColor(newSparsePCColor);
        labWidget.addSparsePCGraph(
            m_Preferences, component,
            generateGraphName(currentSparseComponentRound,
                              cumulativeVariancePercentage));
        qDebug() << "... sparse candidates added";
    }

    void LabController::onRemoveLastComponent()
    {
        qDebug() << "Removing last component ...";
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();

        if (labWidget.m_StandardPCRadioButton->isChecked())
        {
            if (!labModel.removeLastStandardComponent())
            {
                qDebug() << "... no standard component removed";
                return;
            }

            labWidget.removeLastStandardPCGraph();
            qDebug() << "... last standard component removed";
            return;
        }

        if (!labModel.removeLastSparsePC())
        {
            qDebug() << "... no sparse component removed";
            return;
        }

        labWidget.removeLastSparsePCGraph(labModel.getiWinner());
        qDebug() << "... last sparse component removed";
    }

    void LabController::onValueChanged(int value)
    {
        auto &labWidget = m_LabWidget.get();
        auto &labModel = m_LabModel.get();
        if (labModel.m_SparsePCs.empty())
        {
            return;
        }
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        auto &sparsePC = labModel.m_SparsePCs.back();
        sparsePC.iWinner = value;
        const auto &component = sparsePC.candidates.at(sparsePC.iWinner);
        const auto cumulativeVariancePercentage =
            labModel.computeVarianceRatio(
                labModel
                    .computePreviousSparseComponentRoundsCumulativeVariance() +
                component.value) *
            100.0;
        const auto currentSparseComponentRound =
            labModel.getCurrentSparseComponentRound();
        labWidget.updateLastSparsePCGraph(
            component, generateGraphName(currentSparseComponentRound,
                                         cumulativeVariancePercentage));
    }

    void LabController::onSelectionChanged(int index)
    {
        auto &labWidget = m_LabWidget.get();
        const DeferredUpdateForPlots deferredUpdateForPlots(labWidget);
        labWidget.clearAlls();
        addStandardPCGraphs();
        addSparsePCGraphs();
    }

    bool LabController::saveProject(const QString &fileName) const
    {
        return m_ModelHandler.saveProject(fileName);
    }

    bool LabController::loadProject(const QString &fileName)
    {
        return m_ModelHandler.loadProject(fileName);
    }
} // namespace Sparsely
