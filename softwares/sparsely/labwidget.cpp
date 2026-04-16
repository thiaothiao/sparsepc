#include "labwidget.h"

#include <QColor>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLibrary>
#include <QPen>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QSlider>
#include <QStackedLayout>
#include <QString>
#include <QVBoxLayout>

#include "jkqtplotter/graphs/jkqtpfilledcurve.h"
#include "jkqtplotter/graphs/jkqtpimpulses.h"
#include "jkqtplotter/jkqtplotter.h"

namespace
{
    void addSparselyIconGraphs(JKQTPlotter &plotter)
    {
        auto *datastore = plotter.getDatastore();
        const auto n = 13;
        const auto columnX = datastore->addLinearColumn(n, 0, n - 1);
        sparsepc::Vector<double> redValues =
            sparsepc::Vector<double>::Constant(n, 0.01);
        redValues[3] = 0.25;
        redValues[4] = 0.5;
        redValues[5] = 0.25;
        redValues.normalize();
        sparsepc::Vector<double> greenValues =
            sparsepc::Vector<double>::Constant(n, -0.01);
        greenValues[7] = -0.25;
        greenValues[8] = -0.5;
        greenValues[9] = -0.25;
        greenValues.normalize();

        const auto columnRed = datastore->addColumn(n);
        const auto columnGreen = datastore->addColumn(n);
        datastore->setAll(columnRed, static_cast<double>(0));
        for (int i = 0; i < n; ++i)
        {
            datastore->inc(columnRed, i, redValues[i]);
            datastore->inc(columnGreen, i, greenValues[i]);
        }

        {
            auto *redGraph = new JKQTPFilledCurveXGraph(&plotter);
            auto col = QColor(QColor("red"));
            redGraph->setLineStyle(Qt::SolidLine);
            redGraph->setLineWidth(10);
            redGraph->setLineColor(col);
            redGraph->setFillMode(
                JKQTPFilledCurveXGraph::FillMode::SingleFilling);
            col.setAlphaF(0.125f);
            redGraph->setFillColor(col);
            redGraph->fillStyleBelow().setFillColor(col);
            redGraph->setBaseline(0.0);
            redGraph->setXColumn(columnX);
            redGraph->setYColumn(columnRed);
            redGraph->setTitle("0");
            plotter.addGraph(redGraph);
        }
        {
            auto *greenGraph = new JKQTPFilledCurveXGraph(&plotter);
            auto col = QColor(QColor("green"));
            greenGraph->setLineStyle(Qt::SolidLine);
            greenGraph->setLineWidth(10);
            greenGraph->setLineColor(col);
            greenGraph->setFillMode(
                JKQTPFilledCurveXGraph::FillMode::SingleFilling);
            col.setAlphaF(0.125f);
            greenGraph->setFillColor(col);
            greenGraph->fillStyleBelow().setFillColor(col);
            greenGraph->setBaseline(0.0);
            greenGraph->setXColumn(columnX);
            greenGraph->setYColumn(columnGreen);
            greenGraph->setTitle("1");
            plotter.addGraph(greenGraph);
        }
    }
    void clear(JKQTPlotter &plotter)
    {
        plotter.clearGraphs();
        auto *datastore = plotter.getDatastore();
        if (datastore)
        {
            datastore->clear();
        }
    }
} // namespace

namespace sparsely
{
    LabWidget::LabWidget(QWidget *parent)
        : QWidget(parent), m_Plotter{nullptr}, m_SliderGroupBox{nullptr},
          m_Slider{nullptr}, m_SliderOrProgressBarWidgetStackedLayout{nullptr},
          m_MethodComboBox{nullptr}, m_PlotTypeComboBox{nullptr}, m_Colors{},
          m_N{0}, m_ColumnX{0}
    {
    }

    void LabWidget::addPCGraph(std::vector<GraphInfo> &pcs,
                               const sparsepc::Component<double> &component,
                               const QString &legend,
                               const Qt::PenStyle &lineStyle,
                               double lineWidthFilledPlot,
                               double lineWidthImpulsesPlot,
                               double fillingColorsAlpha)
    {
        auto &pCGraph = pcs.emplace_back();
        pCGraph.color = m_Colors[pcs.size() - 1];
        JKQTPDatastore *ds = m_Plotter->getDatastore();
        pCGraph.pcColumn = ds->addColumn(m_N);
        pCGraph.xColumn = m_ColumnX;
        ds->setAll(pCGraph.pcColumn, static_cast<double>(0));
        for (int i = 0; i < m_N; ++i)
        {
            ds->inc(pCGraph.pcColumn, i, component.vector[i]);
        }
        const auto plotType =
            m_PlotTypeComboBox->currentData().value<Enums::PlotType>();
        switch (plotType)
        {
        case Enums::PlotType::FILLED: {
            auto *graph = new JKQTPFilledCurveXGraph(m_Plotter);
            auto col = QColor(pCGraph.color);
            graph->setLineStyle(lineStyle);
            graph->setLineWidth(lineWidthFilledPlot);
            graph->setLineColor(col);
            graph->setFillMode(JKQTPFilledCurveXGraph::FillMode::SingleFilling);
            col.setAlphaF(fillingColorsAlpha);
            graph->setFillColor(col);
            graph->fillStyleBelow().setFillColor(col);
            graph->setBaseline(0.0);
            graph->setXColumn(pCGraph.xColumn);
            graph->setYColumn(pCGraph.pcColumn);
            pCGraph.graph = graph;
            break;
        }
        case Enums::PlotType::IMPULSES: {
            auto *graph = new JKQTPImpulsesVerticalGraph(m_Plotter);
            graph->setLineStyle(lineStyle);
            graph->setLineWidth(lineWidthImpulsesPlot);
            graph->setDrawSymbols(true);
            graph->setSymbolType(
                lineStyle == Qt::SolidLine
                    ? JKQTPGraphSymbols::JKQTPFilledCircle
                    : JKQTPGraphSymbols::JKQTPCircle); // allowed 2 symbol types
            auto col = QColor(pCGraph.color);
            graph->setColor(col);
            graph->setXColumn(pCGraph.xColumn);
            graph->setYColumn(pCGraph.pcColumn);
            pCGraph.graph = graph;
            break;
        }
        default: {
            break;
        }
        }

        pCGraph.graph->setTitle(legend);
        m_Plotter->addGraph(pCGraph.graph);
    }

    void
    LabWidget::addStandardPCGraph(const Preferences &preferences,
                                  const sparsepc::Component<double> &component,
                                  const QString &legend)
    {
        addPCGraph(m_StandardPCGraphs, component, legend,
                   preferences.standardComponentsLineStyle,
                   preferences.standardComponentsLineWidthFilledPlot,
                   preferences.standardComponentsLineWidthImpulsesPlot,
                   preferences.standardComponentsFillingColorsAlpha);
    }

    void
    LabWidget::addSparsePCGraph(const Preferences &preferences,
                                const sparsepc::Component<double> &component,
                                const QString &legend)
    {
        addPCGraph(m_SparsePCGraphs, component, legend,
                   preferences.sparseComponentsLineStyle,
                   preferences.sparseComponentsLineWidthFilledPlot,
                   preferences.sparseComponentsLineWidthImpulsesPlot,
                   preferences.sparseComponentsFillingColorsAlpha);
    }

    void LabWidget::updateLastSparsePCGraph(
        const sparsepc::Component<double> &component, const QString &legend)
    {
        if (m_SparsePCGraphs.empty())
        {
            return;
        }
        auto &sparsePCGraph = m_SparsePCGraphs.back();
        JKQTPDatastore *ds = m_Plotter->getDatastore();
        ds->setAll(sparsePCGraph.pcColumn, static_cast<double>(0));
        for (int i = 0; i < m_N; ++i)
        {
            ds->inc(sparsePCGraph.pcColumn, i, component.vector[i]);
        }
        sparsePCGraph.graph->setTitle(legend);
        m_Plotter->redrawPlot();
    }

    void LabWidget::removeLastSparsePCGraph(int sliderValue)
    {
        if (m_SparsePCGraphs.empty())
        {
            return;
        }
        m_Plotter->deleteGraph(m_SparsePCGraphs.back().graph, true);
        m_Plotter->getDatastore()->deleteColumn(
            m_SparsePCGraphs.back().pcColumn, true);
        if (!m_SparsePCGraphs.empty())
        {
            m_SparsePCGraphs.pop_back();
        }
        if (!m_SparsePCGraphs.empty())
        {
            setSliderColor(m_SparsePCGraphs.back().color);
            m_Slider->setValue(sliderValue);
        }
        else
        {
            setSliderColor("blue");
        }
    }

    void LabWidget::clearAlls()
    {
        // m_PlotTypeComboBox->currentText();
        for (auto &spcg : m_SparsePCGraphs)
        {
            m_Plotter->deleteGraph(spcg.graph, true);
            m_Plotter->getDatastore()->deleteColumn(spcg.pcColumn, true);
            spcg.graph = nullptr;
        }
        m_SparsePCGraphs.clear();
        for (auto &spcg : m_StandardPCGraphs)
        {
            m_Plotter->deleteGraph(spcg.graph, true);
            m_Plotter->getDatastore()->deleteColumn(spcg.pcColumn, true);
            spcg.graph = nullptr;
        }
        m_StandardPCGraphs.clear();
    }
    void LabWidget::welcome(const QString &addonName)
    {
        auto *layout = new QGridLayout(this);
        auto *plotGroupbox = new QGroupBox(this);
        layout->addWidget(plotGroupbox, 0, 0);
        auto *plotGroupboxLayout = new QHBoxLayout(plotGroupbox);
        m_Plotter = new JKQTPlotter(plotGroupbox);
        addSparselyIconGraphs(*m_Plotter);
        m_Plotter->zoomToFit();
        m_Plotter->resize(400, 300);
        plotGroupboxLayout->addWidget(m_Plotter);
        // m_Plotter->redrawPlot();

        auto *sliderOrProgressWidget = new QWidget(this);
        layout->addWidget(sliderOrProgressWidget, 1, 0);
        m_SliderOrProgressBarWidgetStackedLayout =
            new QStackedLayout(sliderOrProgressWidget);
        m_SliderOrProgressBarWidgetStackedLayout->setStackingMode(
            QStackedLayout::StackOne);
        m_SliderGroupBox = new QGroupBox("1", this);
        auto *sliderGroupBoxLayout = new QHBoxLayout(m_SliderGroupBox);
        m_Slider = new QSlider(Qt::Orientation::Horizontal, this);
        sliderGroupBoxLayout->addWidget(m_Slider);
        m_Slider->setRange(1, 12);
        m_Slider->setSingleStep(1);
        m_Slider->setMaximumHeight(18);
        setSliderColor("blue");

        auto *welcomeGroupBox = new QGroupBox("Welcome", this);
        auto *welcomeGroupBoxLayout = new QHBoxLayout(welcomeGroupBox);
        auto *topLevelLabel =
            new QLabel("Here you will be a slider allowing the tuning of the "
                       "sparsity level",
                       this);
        welcomeGroupBoxLayout->addWidget(topLevelLabel, 0, Qt::AlignCenter);

        m_SliderOrProgressBarWidgetStackedLayout->addWidget(m_SliderGroupBox);
        m_SliderOrProgressBarWidgetStackedLayout->addWidget(welcomeGroupBox);
        m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(
            welcomeGroupBox);
        auto *processingsGoupbox = new QGroupBox(this);
        layout->addWidget(processingsGoupbox, 2, 0);
        layout->setRowStretch(0, 12);
        layout->setRowStretch(1, 1);
        layout->setRowStretch(2, 1);
        m_ProcessingsGoupboxStackedLayout =
            new QStackedLayout(processingsGoupbox);
        m_ProcessingsGoupboxStackedLayout->setStackingMode(
            QStackedLayout::StackOne);

        auto *buttonsWelcomeWidget = new QWidget(this);
        m_ProcessingsGoupboxStackedLayout->addWidget(buttonsWelcomeWidget);
        auto *buttonsWelcomeWidgetLayout =
            new QHBoxLayout(buttonsWelcomeWidget);
        auto *buttonsWidgetWelcomeLabel =
            new QLabel("Click on File to create or load a project from a "
                       "convariance matrix.\n"
                       "And then compute or select sparse pcs",
                       this);
        buttonsWelcomeWidgetLayout->addWidget(buttonsWidgetWelcomeLabel, 0,
                                              Qt::AlignCenter);

        m_ButtonsWidget = new QWidget(this);
        m_ProcessingsGoupboxStackedLayout->addWidget(m_ButtonsWidget);
        auto *buttonsWidgetLayout = new QHBoxLayout(m_ButtonsWidget);

        auto *plotTypeGroupBox = new QGroupBox(tr("Plot"), this);
        auto *plotTypeGroupBoxLayout = new QHBoxLayout(plotTypeGroupBox);
        m_PlotTypeComboBox = new QComboBox(this);
        m_PlotTypeComboBox->addItem(
            tr("Filled"), QVariant::fromValue(Enums::PlotType::FILLED));
        m_PlotTypeComboBox->addItem(
            tr("Impulses"), QVariant::fromValue(Enums::PlotType::IMPULSES));
        plotTypeGroupBoxLayout->addWidget(m_PlotTypeComboBox);
        buttonsWidgetLayout->addWidget(plotTypeGroupBox);
        auto *methodGroupBox = new QGroupBox(tr("Method"), this);
        auto *methodGroupBoxLayout = new QHBoxLayout(methodGroupBox);
        m_MethodComboBox = new QComboBox(this);
        m_MethodComboBox->addItem(tr("Dca"),
                                  QVariant::fromValue(Enums::Method::DCA));
        m_MethodComboBox->addItem(tr("Forward Gspca"),
                                  QVariant::fromValue(Enums::Method::FGSPCA));
        m_MethodComboBox->addItem(tr("Backward Gspca"),
                                  QVariant::fromValue(Enums::Method::BGSPCA));
        m_MethodComboBox->addItem(tr("Custom"),
                                  QVariant::fromValue(Enums::Method::CUSTOM));
        if (!addonName.isEmpty())
        {
            m_MethodComboBox->addItem(
                addonName, QVariant::fromValue(Enums::Method::USERDYNAMICLIB));
        }
        methodGroupBoxLayout->addWidget(m_MethodComboBox);
        buttonsWidgetLayout->addWidget(methodGroupBox);
        auto *actionGroupBox = new QGroupBox(tr("Sparse component"), this);
        auto *actionGroupBoxLayout = new QHBoxLayout(actionGroupBox);
        buttonsWidgetLayout->addWidget(actionGroupBox);
        m_AddNewSparseComponentButton = new QPushButton(tr("Add new"), this);
        actionGroupBoxLayout->addWidget(m_AddNewSparseComponentButton);
        m_RemoveLastSparseComponentButton =
            new QPushButton(tr("Remove last"), this);
        actionGroupBoxLayout->addWidget(m_RemoveLastSparseComponentButton);
        // QObject::connect(m_Slider, qOverload<int>(&QSlider::valueChanged),
        //                  this, &LabWidget::updateSliderTitle);
        buttonsWidgetLayout->addStretch(1);

        m_ProcessingsGoupboxStackedLayout->setCurrentWidget(
            buttonsWelcomeWidget);
    }

    void LabWidget::updateWidget(const Preferences &preferences,
                                 const QString &addonName)
    {
        m_Colors.reserve(preferences.componentsColors.size());
        foreach (const auto &colorString, preferences.componentsColors)
        {
            m_Colors.push_back(colorString);
        }
        m_StandardPCGraphs.reserve(preferences.componentsColors.size());
        m_SparsePCGraphs.reserve(preferences.componentsColors.size());
        m_Plotter->setPlotUpdateEnabled(false);
        clear(*m_Plotter);
        m_Plotter->setPlotUpdateEnabled(true);
        // m_Plotter->redrawPlot();
        auto *datastore = m_Plotter->getDatastore();
        m_ColumnX = datastore->addLinearColumn(m_N, 0, m_N - 1);
        m_Slider->setRange(1, m_N);
        m_Slider->setSingleStep(1);
        setSliderColor("blue");
        m_PlotTypeComboBox->setCurrentIndex(
            preferences.plotType == Enums::PlotType::FILLED ? 0 : 1);
        switch (preferences.method)
        { // should use a map type ontainer!
        case Enums::Method::DCA: {
            m_MethodComboBox->setCurrentIndex(0);
            break;
        }
        case Enums::Method::FGSPCA: {
            m_MethodComboBox->setCurrentIndex(1);
            break;
        }
        case Enums::Method::BGSPCA: {
            m_MethodComboBox->setCurrentIndex(2);
            break;
        }
        case Enums::Method::CUSTOM: {
            m_MethodComboBox->setCurrentIndex(3);
            break;
        }
        default: {
            m_MethodComboBox->setCurrentIndex(0);
            break;
        }
        }
        if (!addonName.isEmpty() &&
            preferences.method == Enums::Method::USERDYNAMICLIB)
        {
            m_MethodComboBox->setCurrentIndex(4);
        }
        m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(
            m_SliderGroupBox);
        m_ProcessingsGoupboxStackedLayout->setCurrentWidget(m_ButtonsWidget);
        // auto* actionGroupBox->setTitle(tr("Sparse component"));
        // m_AddNewSparseComponentButton->setText(tr("Add new"));
        // m_RemoveLastSparseComponentButton->setText((tr("Remove last"));
        QObject::connect(m_Slider, qOverload<int>(&QSlider::valueChanged), this,
                         &LabWidget::updateSliderTitle);
    }

    void LabWidget::setPlotUpdateEnabled(bool enable)
    {
        m_Plotter->setPlotUpdateEnabled(enable);
    }

    void LabWidget::redrawPlot() { m_Plotter->redrawPlot(); }

    void LabWidget::zoomToFit()
    {
        m_Plotter->setAbsoluteX(static_cast<double>(0),
                                static_cast<double>(m_N - 1));
        m_Plotter->setAbsoluteY(static_cast<double>(-1),
                                static_cast<double>(1));
        m_Plotter->zoomToFit();
        // m_Plotter->resize(400,300);
    }

    void LabWidget::reInitSlider(int value)
    {
        if (!m_SparsePCGraphs.empty())
        {
            setSliderColor(m_SparsePCGraphs.back().color);
        }
        m_Slider->setValue(value);
    }

    void LabWidget::updateSliderTitle(int value)
    {
        m_SliderGroupBox->setTitle(QString::number(value));
    }

    void LabWidget::setN(int n) { m_N = n; }

    void LabWidget::setSliderColor(const QString &colorString)
    {
        m_SliderGroupBox->setStyleSheet(
            "QGroupBox::title { color: " + colorString + "; }");
        m_Slider->setStyleSheet(
            "QSlider::groove:horizontal {"
            "    border: 1px solid #999;"
            "    background: #eee;"
            "    height: 2px;"
            "}"
            "QSlider::handle:horizontal {"
            "    background: " +
            colorString +
            ";"
            "    width: 9px;"
            "    height: 18px;"
            "    margin: -7px 0;" // Pulls handle outside groove
            "}"
            "QSlider::sub-page:horizontal {"
            "    background: " +
            colorString +
            ";"
            "}");
    }
} // namespace sparsely
