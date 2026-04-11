#include "labwidget.h"

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
#include <QDir>
#include <QDebug>

#include "jkqtplotter/graphs/jkqtpfilledcurve.h"
#include "jkqtplotter/graphs/jkqtpimpulses.h"

namespace sparsely
{
    LabWidget::LabWidget(QWidget* parent)
        : QWidget(parent), m_Plotter{nullptr},
        m_SliderGroupBox{nullptr}, m_ProgressBarGroupBox{nullptr},
        m_Slider{nullptr}, m_ProgressBar{nullptr}, m_Preferences{nullptr},
        m_SliderOrProgressBarWidgetStackedLayout{nullptr},
        m_MethodComboBox{nullptr}, m_PlotTypeComboBox{nullptr}, m_Colors{},
        m_N{0}, m_ColumnX{0}
    {
    }

    void LabWidget::addPCGraph(std::vector<MyClass2>& pcs, const sparsepc::Component<double>& component,
        const QString& cumulativeVarianceString, const QString& curveName,
        const Qt::PenStyle& lineStyle, double lineWidthFilledPlot,
        double lineWidthImpulsesPlot, double fillingColorsAlpha)
    {
        auto& pCGraph = pcs.emplace_back();
        pCGraph.color = m_Colors[pcs.size()-1];
        JKQTPDatastore* ds = m_Plotter->getDatastore();
        pCGraph.pcColumn = ds->addColumn(m_N, curveName);
        pCGraph.xColumn = m_ColumnX;
        ds->setAll(pCGraph.pcColumn, static_cast<double>(0));
        for (int i=0; i< m_N; ++i)
        {
            ds->inc(pCGraph.pcColumn, i, component.vector[i]);
        }
        const auto plotType = m_PlotTypeComboBox->currentData().value<Enums::PlotType>();
        switch (plotType)
        {
        case Enums::PlotType::FILLED:
        {
            auto* graph = new JKQTPFilledCurveXGraph(m_Plotter);
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
        case Enums::PlotType::IMPULSES:
        {
            auto* graph = new JKQTPImpulsesVerticalGraph(m_Plotter);
            graph->setLineStyle(lineStyle);
            graph->setLineWidth(lineWidthImpulsesPlot);
            graph->setDrawSymbols(true);
            graph->setSymbolType(JKQTPGraphSymbols::JKQTPFilledCircle);
            auto col = QColor(pCGraph.color);
            graph->setColor(col);
            graph->setXColumn(pCGraph.xColumn);
            graph->setYColumn(pCGraph.pcColumn);
            pCGraph.graph = graph;
            break;
        }
        default:
        {
            break;
        }
        }

        pCGraph.graph->setTitle(curveName + ": " + cumulativeVarianceString);
        m_Plotter->addGraph(pCGraph.graph);
    }

    void LabWidget::addStandardPCGraph(const sparsepc::Component<double>& component,
        const QString& cumulativeVarianceString, const QString& curveName)
    {
        addPCGraph(m_StandardPCGraphs, component, cumulativeVarianceString, curveName,
            m_Preferences->standardComponentsLineStyle,
            m_Preferences->standardComponentsLineWidthFilledPlot,
            m_Preferences->standardComponentsLineWidthImpulsesPlot,
            m_Preferences->standardComponentsFillingColorsAlpha);
    }

    void LabWidget::addSparsePCGraph(const sparsepc::Component<double>& component,
        const QString& cumulativeVarianceString, const QString& curveName)
    {
        addPCGraph(m_SparsePCGraphs, component, cumulativeVarianceString, curveName,
            m_Preferences->sparseComponentsLineStyle,
            m_Preferences->sparseComponentsLineWidthFilledPlot,
            m_Preferences->sparseComponentsLineWidthImpulsesPlot,
            m_Preferences->sparseComponentsFillingColorsAlpha);
    }

    void LabWidget::updateLastSparsePCGraph(const sparsepc::Component<double>& component,
        const QString& cumulativeVarianceString, const QString& curveName)
    {
        if (m_SparsePCGraphs.empty())
        {
            return;
        }
        auto& sparsePCGraph = m_SparsePCGraphs.back();
        JKQTPDatastore* ds = m_Plotter->getDatastore();
        ds->setAll(sparsePCGraph.pcColumn, static_cast<double>(0));
        for (int i=0; i<m_N; ++i)
        {
            ds->inc(sparsePCGraph.pcColumn, i, component.vector[i]);
        }
        sparsePCGraph.graph->setTitle(curveName + ": " + cumulativeVarianceString);
        m_Plotter->redrawPlot();
    }

    void LabWidget::removeLastSparsePCGraph(int sliderValue)
    {
        if(m_SparsePCGraphs.empty())
        {
            return;
        }
        m_Plotter->deleteGraph(m_SparsePCGraphs.back().graph, true);
        m_Plotter->getDatastore()->deleteColumn(m_SparsePCGraphs.back().pcColumn, true);
        if(!m_SparsePCGraphs.empty())
        {
            m_SparsePCGraphs.pop_back();
        }
        if(!m_SparsePCGraphs.empty())
        {
            setSliderColor(m_SparsePCGraphs.back().color);
            m_Slider->setValue(sliderValue);
        }
        else
        {
            setSliderColor("blue");
            m_ProgressBarGroupBox->setStyleSheet("");
        }
    }

    void LabWidget::clearAlls()
    {
        //m_PlotTypeComboBox->currentText();
        for(auto& spcg: m_SparsePCGraphs)
        {
            m_Plotter->deleteGraph(spcg.graph, true);
            m_Plotter->getDatastore()->deleteColumn(spcg.pcColumn, true);
            spcg.graph = nullptr;
        }
        m_SparsePCGraphs.clear();
        for(auto& spcg: m_StandardPCGraphs)
        {
            m_Plotter->deleteGraph(spcg.graph, true);
            m_Plotter->getDatastore()->deleteColumn(spcg.pcColumn, true);
            spcg.graph = nullptr;
        }
        m_StandardPCGraphs.clear();
    }

    void LabWidget::setPreferences(Preferences* preferences)
    {
        m_Preferences = preferences;
    }

    void LabWidget::createWidget(const QString& addonName)
    {
        m_Colors.reserve(m_Preferences->componentsColors.size());
        //for(const auto& colorString: m_Preferences->componentsColors)
        foreach(const auto& colorString, m_Preferences->componentsColors)
        {
            m_Colors.push_back(colorString);
        }
        auto* layout = new QGridLayout(this);
        auto* plotGroupbox = new QGroupBox(this);
        layout->addWidget(plotGroupbox, 0, 0);
        auto* plotGroupboxLayout = new QHBoxLayout(plotGroupbox);
        m_Plotter = new JKQTPlotter();
        m_Plotter->setPlotUpdateEnabled(false);

        //m_Plotter->getPlotter()->setUseAntiAliasingForGraphs(true); // nicer (but slower) plotting
        //m_Plotter->getPlotter()->setUseAntiAliasingForSystem(true); // nicer (but slower) plotting
        //m_Plotter->getPlotter()->setUseAntiAliasingForText(true); // nicer (but slower) text rendering

        m_StandardPCGraphs.reserve(m_Preferences->componentsColors.size());
        m_SparsePCGraphs.reserve(m_Preferences->componentsColors.size());
        JKQTPDatastore* datastore = m_Plotter->getDatastore();
        m_ColumnX = datastore->addLinearColumn(m_N, 0, m_N-1);
        plotGroupboxLayout->addWidget(m_Plotter);
        auto* sliderOrProgressWidget = new QWidget(this);
        layout->addWidget(sliderOrProgressWidget, 1, 0);
        m_SliderOrProgressBarWidgetStackedLayout = new QStackedLayout(sliderOrProgressWidget);
        m_SliderOrProgressBarWidgetStackedLayout->setStackingMode(QStackedLayout::StackOne);
        m_SliderGroupBox = new QGroupBox("1", this);
        auto* sliderGroupBoxLayout = new QHBoxLayout(m_SliderGroupBox);
        m_Slider = new QSlider(Qt::Orientation::Horizontal, this);
        sliderGroupBoxLayout->addWidget(m_Slider);
        m_Slider->setRange(1, m_N);
        m_Slider->setSingleStep(1);
        m_Slider->setMaximumHeight(18);
        setSliderColor("blue");
        m_ProgressBarGroupBox = new QGroupBox("0%", this);
        auto* progressBarGroupBoxLayout = new QHBoxLayout(m_ProgressBarGroupBox);
        m_ProgressBar = new QProgressBar(this);
        progressBarGroupBoxLayout->addWidget(m_ProgressBar);
        m_ProgressBar->setRange(0, m_N);
        m_ProgressBar->setValue(0);
        m_ProgressBar->setTextVisible(false);
        m_ProgressBar->setMaximumHeight(10);
        m_SliderOrProgressBarWidgetStackedLayout->addWidget(m_SliderGroupBox);
        m_SliderOrProgressBarWidgetStackedLayout->addWidget(m_ProgressBarGroupBox);
        m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_SliderGroupBox);
        auto* processingsGoupbox = new QGroupBox(this);
        layout->addWidget(processingsGoupbox, 2, 0);
        layout->setRowStretch(0, 12);
        layout->setRowStretch(1, 1);
        layout->setRowStretch(2, 1);
        auto* processingsGoupboxLayout = new QHBoxLayout(processingsGoupbox);
        auto* plotTypeGroupBox = new QGroupBox(tr("Plot"), this);
        auto* plotTypeGroupBoxLayout = new QHBoxLayout(plotTypeGroupBox);
        m_PlotTypeComboBox = new QComboBox(this);
        m_PlotTypeComboBox->addItem(tr("Filled"), QVariant::fromValue(Enums::PlotType::FILLED));
        m_PlotTypeComboBox->addItem(tr("Impulses"), QVariant::fromValue(Enums::PlotType::IMPULSES));
        m_PlotTypeComboBox->setCurrentIndex(m_Preferences->plotType == Enums::PlotType::FILLED ? 0 : 1);
        plotTypeGroupBoxLayout->addWidget(m_PlotTypeComboBox);
        processingsGoupboxLayout->addWidget(plotTypeGroupBox);
        auto* methodGroupBox = new QGroupBox(tr("Method"), this);
        auto* methodGroupBoxLayout = new QHBoxLayout(methodGroupBox);
        m_MethodComboBox = new QComboBox(this);
        m_MethodComboBox->addItem(tr("Dca"), QVariant::fromValue(Enums::Method::DCA));
        m_MethodComboBox->addItem(tr("Forward Gspca"), QVariant::fromValue(Enums::Method::FGSPCA));
        m_MethodComboBox->addItem(tr("Backward Gspca"), QVariant::fromValue(Enums::Method::BGSPCA));
        m_MethodComboBox->addItem(tr("Custom"), QVariant::fromValue(Enums::Method::CUSTOM));
        if(!addonName.isEmpty())
        {
            m_MethodComboBox->addItem(addonName, QVariant::fromValue(Enums::Method::USERDYNAMICLIB));
        }
        switch (m_Preferences->method)
        {// should use a map type ontainer!
        case Enums::Method::DCA:
        {
            m_MethodComboBox->setCurrentIndex(0);
            break;
        }
        case Enums::Method::FGSPCA:
        {
            m_MethodComboBox->setCurrentIndex(1);
            break;
        }
        case Enums::Method::BGSPCA:
        {
            m_MethodComboBox->setCurrentIndex(2);
            break;
        }
        case Enums::Method::CUSTOM:
        {
            m_MethodComboBox->setCurrentIndex(3);
            break;
        }
        default:
        {
            m_MethodComboBox->setCurrentIndex(0);
            break;
        }
        }

        if(!addonName.isEmpty() &&
            m_Preferences->method == Enums::Method::USERDYNAMICLIB)
        {
            m_MethodComboBox->setCurrentIndex(4);
        }
        methodGroupBoxLayout->addWidget(m_MethodComboBox);
        processingsGoupboxLayout->addWidget(methodGroupBox);
        auto* actionGroupBox = new QGroupBox(tr("Sparse component"), this);
        auto* actionGroupBoxLayout = new QHBoxLayout(actionGroupBox);
        processingsGoupboxLayout->addWidget(actionGroupBox);
        m_AddNewSparseComponentButton
            = new QPushButton(tr("Add new"), this);
        actionGroupBoxLayout->addWidget(m_AddNewSparseComponentButton);
        m_RemoveLastSparseComponentButton =
            new QPushButton(tr("Remove last"), this);
        actionGroupBoxLayout->addWidget(m_RemoveLastSparseComponentButton);
        QObject::connect(m_ProgressBar, &QProgressBar::valueChanged,
                         this, &LabWidget::updateProgressBarTitle);
        QObject::connect(m_Slider, qOverload<int>(&QSlider::valueChanged),
                         this, &LabWidget::updateSliderTitle);
        processingsGoupboxLayout->addStretch(1);
    }

    void LabWidget::sePlotUpdateEnabled(bool enable)
    {
        m_Plotter->setPlotUpdateEnabled(enable);
    }

    void LabWidget::redrawPlot()
    {
        m_Plotter->redrawPlot();
    }

    void LabWidget::zoomToFit()
    {
        m_Plotter->setAbsoluteX(static_cast<double>(0), static_cast<double>(m_N-1));
        m_Plotter->setAbsoluteY(static_cast<double>(-1), static_cast<double>(1));
        m_Plotter->zoomToFit(true, false);
        m_Plotter->resize(400,300);
    }

    void LabWidget::reInitSlider(int value)
    {
        if(!m_SparsePCGraphs.empty())
        {
            setSliderColor(m_SparsePCGraphs.back().color);
        }
        m_Slider->setValue(value);
    }

    void LabWidget::updateSliderTitle(int value)
    {
        m_SliderGroupBox->setTitle(QString::number(value));
    }

    void LabWidget::setN(int n)
    {
        m_N = n;
    }

    void LabWidget::updateProgressBarTitle(int value)
    {
        const auto minValue = static_cast<double>(m_ProgressBar->minimum());
        const auto maxValue = static_cast<double>(m_ProgressBar->maximum());
        const auto percentage = static_cast<int>( (value - minValue) * 100.0 / (maxValue - minValue));
        m_ProgressBarGroupBox->setTitle(QString::number(percentage)+"%");
    }

    void LabWidget::setProgressBarColor(const QString& colorString)
    {
        m_ProgressBarGroupBox->setStyleSheet("QGroupBox::title { color: "+ colorString + "; }");
        m_ProgressBar->setStyleSheet("QProgressBar::chunk { background-color:"+ colorString + "; }");
    }

    void LabWidget::setSliderColor(const QString& colorString)
    {
        m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ colorString + "; }");
        m_Slider->setStyleSheet(
            "QSlider::groove:horizontal {"
            "    border: 1px solid #999;"
            "    background: #eee;"
            "    height: 2px;"
            "}"
            "QSlider::handle:horizontal {"
            "    background: "+ colorString + ";"
            "    width: 9px;"
            "    height: 18px;"
            "    margin: -7px 0;" // Pulls handle outside groove
            "}"
            "QSlider::sub-page:horizontal {"
            "    background: "+ colorString + ";"
            "}"
        );
    }
}
