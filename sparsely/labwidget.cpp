#include <labwidget.h>

#include <QButtonGroup>
#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLibrary>
#include <QPen>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QStackedLayout>
#include <QString>
#include <QVBoxLayout>
#include <QtLogging>

#include <utility>

#include <labenums.h>
#include <labpreferences.h>

#include <jkqtplotter/graphs/jkqtpfilledcurve.h>
#include <jkqtplotter/graphs/jkqtpimpulses.h>
#include <jkqtplotter/jkqtplotter.h>

#include <sparsepc/core.hpp>

namespace
{
    constexpr QStringView monthDayFormat = u"MM-dd";
    constexpr double labelAngle = 45.0;
    constexpr unsigned int minTicks = 10u;
    constexpr double xLabelFontSize = 10.0;
    constexpr double yLabelFontSize = 10.0;

    void addSparselyIconGraphs(JKQTPlotter &plotter)
    {
        QDate startDate(2025, 1, 1);
        const QDate endDate(2025, 12, 31);
        QVector<double> date;
        while (startDate <= endDate)
        {
            date << QDateTime::fromString(startDate.toString(Qt::ISODate),
                                          Qt::ISODate)
                        .toUTC()
                        .toMSecsSinceEpoch();
            startDate = startDate.addDays(1); // increment by one day
        }
        const auto n = date.size();
        auto *datastore = plotter.getDatastore();
        const auto colDate = datastore->addCopiedColumn(date, "date");

        Sparsepc::Vector<double> redValues =
            Sparsepc::Vector<double>::Constant(n, 0.0);
        const double nOver2 = n / 2.0;
        const double nOver4 = n / 4.0;
        const double threeNOver4 = n * 3.0 / 4.0;
        for (int i = 0; i <= nOver2; ++i)
        {
            redValues[i] = (i < nOver4) ? i / nOver4 : (2.0 - i / nOver4);
        }
        redValues.normalize();
        Sparsepc::Vector<double> greenValues =
            Sparsepc::Vector<double>::Constant(n, 0.0);
        for (int i = nOver2; i < n; ++i)
        {
            greenValues[i] =
                (i <= threeNOver4) ? (2.0 - i / nOver4) : (i / nOver4 - 4.0);
        }
        greenValues.normalize();

        const auto columnRed = datastore->addColumn(n);
        const auto columnGreen = datastore->addColumn(n);
        datastore->setAll(columnRed, static_cast<double>(0));
        datastore->setAll(columnGreen, static_cast<double>(0));
        for (int i = 0; i < n; ++i)
        {
            datastore->inc(columnRed, i, redValues[i]);
            datastore->inc(columnGreen, i, greenValues[i]);
        }

        {
            auto *redGraph = new JKQTPFilledCurveXGraph(&plotter);
            auto col = QColor(QColor("red"));
            redGraph->setLineStyle(Qt::SolidLine);
            redGraph->setLineWidth(5);
            redGraph->setLineColor(col);
            redGraph->setFillMode(
                JKQTPFilledCurveXGraph::FillMode::SingleFilling);
            col.setAlphaF(0.125f);
            redGraph->setFillColor(col);
            redGraph->fillStyleBelow().setFillColor(col);
            redGraph->setBaseline(0.0);
            redGraph->setXColumn(colDate);
            redGraph->setYColumn(columnRed);
            redGraph->setTitle("0");
            plotter.addGraph(redGraph);
        }
        {
            auto *greenGraph = new JKQTPFilledCurveXGraph(&plotter);
            auto col = QColor(QColor("green"));
            greenGraph->setLineStyle(Qt::SolidLine);
            greenGraph->setLineWidth(5);
            greenGraph->setLineColor(col);
            greenGraph->setFillMode(
                JKQTPFilledCurveXGraph::FillMode::SingleFilling);
            col.setAlphaF(0.125f);
            greenGraph->setFillColor(col);
            greenGraph->fillStyleBelow().setFillColor(col);
            greenGraph->setBaseline(0.0);
            greenGraph->setXColumn(colDate);
            greenGraph->setYColumn(columnGreen);
            greenGraph->setTitle("1");
            plotter.addGraph(greenGraph);
        }

        plotter.getXAxis()->clearAxisTickLabels();
        plotter.getXAxis()->setTickLabelType(JKQTPCALTdatetime);
        plotter.getXAxis()->setTickDateTimeFormat(monthDayFormat.toString());
        plotter.getXAxis()->setTickLabelAngle(labelAngle);
        plotter.getXAxis()->setMinTicks(minTicks);
        plotter.getXAxis()->setTickLabelFontSize(xLabelFontSize);
        plotter.getYAxis()->setTickLabelFontSize(yLabelFontSize);
        plotter.setAbsoluteX(date.front(), date.back());
        plotter.setAbsoluteY(-1, 1);
    }

    void clear(JKQTPlotter &plotter)
    {
        plotter.clearGraphs();
        // plotter.getXAxis()->clearAxisTickLabels();
        auto *datastore = plotter.getDatastore();
        if (datastore)
        {
            datastore->clear();
        }
    }
} // namespace

namespace Sparsely
{
    LabWidget::LabWidget(Enums::ScaleType scale, QWidget *parent)
        : QWidget(parent), m_Plotter{nullptr}, m_SliderGroupBox{nullptr},
          m_Slider{nullptr}, m_SliderOrProgressBarWidgetStackedLayout{nullptr},
          m_MethodComboBox{nullptr}, m_PlotTypeComboBox{nullptr}, m_Colors{},
          m_N{0}, m_ColumnX{0}, m_Xmin{0.0}, m_Xmax{1.0}, m_Ymin{-1.0},
          m_Ymax{1.0}, m_Scale{scale}
    {
    }

    void LabWidget::addPCGraph(std::vector<GraphInfo> &pcs,
                               const Sparsepc::Component<double> &component,
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
            graph->setSymbolType(JKQTPGraphSymbols::JKQTPDot);
            auto col = QColor(pCGraph.color);
            graph->setColor(col);
            graph->setXColumn(pCGraph.xColumn);
            graph->setYColumn(pCGraph.pcColumn);
            pCGraph.graph = graph;
            break;
        }
        default:
            break;
        }

        pCGraph.graph->setTitle(legend);
        m_Plotter->addGraph(pCGraph.graph);
    }

    void
    LabWidget::addStandardPCGraph(const Preferences &preferences,
                                  const Sparsepc::Component<double> &component,
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
                                const Sparsepc::Component<double> &component,
                                const QString &legend)
    {
        addPCGraph(m_SparsePCGraphs, component, legend,
                   preferences.sparseComponentsLineStyle,
                   preferences.sparseComponentsLineWidthFilledPlot,
                   preferences.sparseComponentsLineWidthImpulsesPlot,
                   preferences.sparseComponentsFillingColorsAlpha);
    }

    void LabWidget::updateLastSparsePCGraph(
        const Sparsepc::Component<double> &component, const QString &legend)
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
    }

    void LabWidget::removeLastStandardPCGraph()
    {
        if (m_StandardPCGraphs.empty())
        {
            return;
        }
        m_Plotter->deleteGraph(m_StandardPCGraphs.back().graph, true);
        m_Plotter->getDatastore()->deleteColumn(
            m_StandardPCGraphs.back().pcColumn, true);

        m_StandardPCGraphs.pop_back();
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
        m_SparsePCGraphs.pop_back();
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
        qDebug() << "Welcoming ...";
        auto *layout = new QGridLayout(this);
        auto *plotGroupbox = new QGroupBox(this);
        layout->addWidget(plotGroupbox, 0, 0);
        auto *plotGroupboxLayout = new QHBoxLayout(plotGroupbox);
        m_Plotter = new JKQTPlotter(plotGroupbox);
        m_Plotter->getMainKey()->setFrameLineStyle(Qt::NoPen);
        m_Plotter->getMainKey()->setBackgroundBrush(QBrush(Qt::transparent));
        addSparselyIconGraphs(*m_Plotter);
        m_Plotter->zoomToFit();
        m_Plotter->resize(400, 300);
        plotGroupboxLayout->addWidget(m_Plotter);

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
        auto *topLevelLabel = new QLabel("A slider will appear here allowing "
                                         "you to control the sparsity level",
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
                       "features matrix.\n"
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
        m_MethodComboBox->insertItem(std::to_underlying(Enums::Method::DCA),
                                     tr("Dca"),
                                     QVariant::fromValue(Enums::Method::DCA));
        m_MethodComboBox->insertItem(
            std::to_underlying(Enums::Method::FGSPCA), tr("Forward Gspca"),
            QVariant::fromValue(Enums::Method::FGSPCA));
        m_MethodComboBox->insertItem(
            std::to_underlying(Enums::Method::BGSPCA), tr("Backward Gspca"),
            QVariant::fromValue(Enums::Method::BGSPCA));
        m_MethodComboBox->insertItem(
            std::to_underlying(Enums::Method::CUSTOM), tr("Custom"),
            QVariant::fromValue(Enums::Method::CUSTOM));
        if (!addonName.isEmpty())
        {
            m_MethodComboBox->insertItem(
                std::to_underlying(Enums::Method::USERDYNAMICLIB), addonName,
                QVariant::fromValue(Enums::Method::USERDYNAMICLIB));
        }
        methodGroupBoxLayout->addWidget(m_MethodComboBox);
        buttonsWidgetLayout->addWidget(methodGroupBox);
        auto *actionGroupBox = new QGroupBox(tr("Component"), this);
        auto *actionGroupBoxLayout = new QHBoxLayout(actionGroupBox);
        buttonsWidgetLayout->addWidget(actionGroupBox);
        m_StandardPCRadioButton = new QRadioButton(tr("Standard"), this);
        m_SparsePCRadioButton = new QRadioButton(tr("Sparse"), this);
        auto *groupButton = new QButtonGroup(this);
        groupButton->addButton(m_StandardPCRadioButton);
        groupButton->addButton(m_SparsePCRadioButton);
        m_StandardPCRadioButton->setChecked(true);
        actionGroupBoxLayout->addWidget(m_StandardPCRadioButton);
        actionGroupBoxLayout->addWidget(m_SparsePCRadioButton);
        m_AddNewComponentButton = new QPushButton(tr("Add new"), this);
        actionGroupBoxLayout->addWidget(m_AddNewComponentButton);
        m_RemoveLastComponentButton = new QPushButton(tr("Remove last"), this);
        actionGroupBoxLayout->addWidget(m_RemoveLastComponentButton);
        // QObject::connect(m_Slider, qOverload<int>(&QSlider::valueChanged),
        //                  this, &LabWidget::updateSliderTitle);
        buttonsWidgetLayout->addStretch(1);

        m_ProcessingsGoupboxStackedLayout->setCurrentWidget(
            buttonsWelcomeWidget);

        qDebug() << "... welcomed";
    }

    void LabWidget::updateWidget(const Preferences &preferences,
                                 const QString &addonName,
                                 const QVector<QString> &header)
    {
        m_Colors.reserve(preferences.componentsColors.size());
        foreach (const auto &colorString, preferences.componentsColors)
        {
            m_Colors.push_back(colorString);
        }
        m_StandardPCGraphs.reserve(preferences.componentsColors.size());
        m_SparsePCGraphs.reserve(preferences.componentsColors.size());
        // m_Plotter->setPlotUpdateEnabled(false);
        clear(*m_Plotter);

        QVector<double> values;
        values.reserve(header.size());
        bool isDateTime = true;
        auto startDate =
            QDateTime::fromString(header[0], monthDayFormat.toString());
        if (startDate.isValid())
        {
            for (const auto &elmt : header)
            {
                values << QDateTime::fromString(startDate.toString(Qt::ISODate),
                                                Qt::ISODate)
                              .toUTC()
                              .toMSecsSinceEpoch();
                startDate = startDate.addDays(1);
            }
        }
        else
        {
            isDateTime = false;
            values.clear();
            for (int i = 0; i < m_N; ++i)
            {
                values << static_cast<double>(i);
            }
        }

        m_Xmin = values.front();
        m_Xmax = values.back();

        auto *datastore = m_Plotter->getDatastore();
        m_ColumnX = datastore->addCopiedColumn(values.data(), m_N, "values");

        m_Plotter->getXAxis()->clearAxisTickLabels();
        if (isDateTime)
        {
            m_Plotter->getXAxis()->setTickLabelType(JKQTPCALTdatetime);
            m_Plotter->getXAxis()->setTickDateTimeFormat(
                monthDayFormat.toString());
            m_Plotter->getXAxis()->setMinTicks(minTicks);
        }
        m_Plotter->getXAxis()->setTickLabelAngle(labelAngle);
        m_Plotter->getXAxis()->setTickLabelFontSize(xLabelFontSize);
        m_Plotter->getYAxis()->setTickLabelFontSize(yLabelFontSize);
        // m_Plotter->setPlotUpdateEnabled(true);

        m_Slider->setRange(1, m_N);
        m_Slider->setSingleStep(1);
        setSliderColor("black");
        m_PlotTypeComboBox->setCurrentIndex(
            std::to_underlying(preferences.plotType));
        if (preferences.method != Enums::Method::USERDYNAMICLIB)
        {
            m_MethodComboBox->setCurrentIndex(
                std::to_underlying(preferences.method));
        }
        else
        {
            m_MethodComboBox->setCurrentIndex(std::to_underlying(
                addonName.isEmpty() ? Enums::Method::DCA
                                    : Enums::Method::USERDYNAMICLIB));
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
        m_Plotter->setAbsoluteX(m_Xmin, m_Xmax);
        m_Plotter->setAbsoluteY(m_Ymin, m_Ymax);
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
} // namespace Sparsely
