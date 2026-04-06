#include "AtelierWidget.h"

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
        QDir path(QDir::currentPath() + "/plugins");

        qDebug() << "current path is: " << path.currentPath();

        QStringList filters;
        filters << "*.dll" << "*.so" << "*.dylib";

        return path.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    }
}

void AtelierWidget::computeStandardPCs()
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
        standardPC.color = m_Colors[j];
        auto& component = components[j];
        component.state = sparsepc::ComponentState::Validated;
        standardPC.candidates.emplace(m_N, std::move(components[j]));
    }
}

void AtelierWidget::drawStandardPCs()
{
    JKQTPDatastore* ds = m_Plotter->getDatastore();

    m_CummulativeVarianceStandardPCs = static_cast<double>(0);

    for (int j=0; j< m_StandardPCs.size(); ++j)
    {
        auto& standardPC = m_StandardPCs[j];

        auto& component = standardPC.candidates.at(standardPC.iCandidate);

        m_CummulativeVarianceStandardPCs += component.value;

        QString formattedValueStr = QString::number(m_CummulativeVarianceStandardPCs, 'f', 2);
        QString formattedPCNumberStr = QString::number(j);
        QString curveName = formattedPCNumberStr;
        standardPC.pcColumn = ds->addColumn(m_N, curveName);

        ds->setAll(standardPC.pcColumn, static_cast<double>(0));

        for (int i=0; i< m_N; ++i)
        {
            ds->inc(standardPC.pcColumn, i, component.vector[i]);
        }
        const auto plotType = m_PlotTypeComboBox->currentData().value<PlotType>();
        switch (plotType)
        {
        case PlotType::FILLED:
        {
            auto* graph = new JKQTPFilledCurveXGraph(m_Plotter);

            auto col = QColor(standardPC.color);
            graph->setLineStyle(Qt::SolidLine);
            graph->setLineWidth(1);
            graph->setLineColor(col);

            graph->setFillMode(JKQTPFilledCurveXGraph::FillMode::SingleFilling);

            col.setAlphaF(0.125f);
            graph->setFillColor(col);
            graph->fillStyleBelow().setFillColor(col);
            graph->setBaseline(0.0);

            graph->setXColumn(m_ColumnX);
            graph->setYColumn(standardPC.pcColumn);

            standardPC.pcGraph = graph;
            break;
        }
        case PlotType::IMPULSES:
        {
            auto* graph = new JKQTPImpulsesVerticalGraph(m_Plotter);
            graph->setLineWidth(2);
            graph->setDrawSymbols(true);
            graph->setSymbolType(JKQTPGraphSymbols::JKQTPFilledCircle);
            auto col = QColor(standardPC.color);
            graph->setColor(col);

            graph->setXColumn(m_ColumnX);
            graph->setYColumn(standardPC.pcColumn);

            standardPC.pcGraph = graph;
            break;
        }
        default:
        {
            break;
        }
        }

        standardPC.pcGraph->setTitle(curveName + ": " + formattedValueStr);

        m_Plotter->addGraph(standardPC.pcGraph);
    }
}

void AtelierWidget::drawSparsePCs()
{
    JKQTPDatastore* ds = m_Plotter->getDatastore();
    m_CummulativeVarianceSparsePCs = static_cast<double>(0);
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

        QString formattedValueStr = QString::number(cummulativeVariance, 'f', 2);
        QString formattedPCNumberStr = QString::number(j);
        QString curveName = formattedPCNumberStr;
        sparsePC.pcColumn = ds->addColumn(m_N, curveName);

        ds->setAll(sparsePC.pcColumn, static_cast<double>(0));

        for (int i=0; i<m_N; ++i)
        {
            ds->inc(sparsePC.pcColumn, i, component.vector[i]);
        }

        const auto plotType = m_PlotTypeComboBox->currentData().value<PlotType>();
        switch (plotType)
        {
        case PlotType::FILLED:
        {
            auto* graph = new JKQTPFilledCurveXGraph(m_Plotter);

            auto col = QColor(sparsePC.color);
            graph->setLineStyle(Qt::DotLine);
            graph->setLineWidth(2);
            graph->setLineColor(col);

            graph->setFillMode(JKQTPFilledCurveXGraph::FillMode::SingleFilling);

            col.setAlphaF(0.25f);
            graph->setFillColor(col);
            graph->fillStyleBelow().setFillColor(col);
            graph->setBaseline(0.0);

            graph->setXColumn(m_ColumnX);
            graph->setYColumn(sparsePC.pcColumn);

            sparsePC.pcGraph = graph;
            break;
        }
        case PlotType::IMPULSES:
        {
            auto* graph = new JKQTPImpulsesVerticalGraph(m_Plotter);
            graph->setLineStyle(Qt::DotLine);
            graph->setLineWidth(3);
            graph->setDrawSymbols(true);
            graph->setSymbolType(JKQTPGraphSymbols::JKQTPCircle);
            auto col = QColor(sparsePC.color);
            graph->setColor(col);

            graph->setXColumn(m_ColumnX);
            graph->setYColumn(sparsePC.pcColumn);

            sparsePC.pcGraph = graph;
            break;
        }
        default:
        {
            break;
        }
        }

        sparsePC.pcGraph->setTitle(curveName + ": " + formattedValueStr);

        m_Plotter->addGraph(sparsePC.pcGraph);
    }
}

void AtelierWidget::clearAllPlots()
{

}

void AtelierWidget::onAddNewSparseComponent()
{
    if(!m_SparsePCs.empty())
    {// validate current selected sparse component
        auto& candidates = m_SparsePCs.back().candidates;
        const auto iCandidate = m_SparsePCs.back().iCandidate;

        m_ValidatedComponents.push_back(candidates.at(iCandidate));
        m_ValidatedComponents.back().get().state = sparsepc::ComponentState::Validated;
        m_CummulativeVarianceSparsePCs += m_ValidatedComponents.back().get().value;
    }

    m_SparsePCs.emplace_back();

    auto& sparsePC = m_SparsePCs.back();

    sparsePC.iCandidate = -1;

    sparsePC.color = m_Colors[m_ValidatedComponents.size()];

    m_ProgressBarGroupBox->setStyleSheet("QGroupBox::title { color: "+ sparsePC.color + "; }");

    m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_ProgressBarGroupBox);

    m_ProgressBar->setValue(0);

    const auto method = m_MethodComboBox->currentData().value<SpcaMethod>();
    switch (method)
    {
    case SpcaMethod::DCA:
    {
        sparsePC.candidates = DCA::computeNextComponentCandidates(
            m_Sigma, DCA::ModelParam(1), m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::BGSPCA:
    {
        sparsePC.candidates = BackwardGSPA::computeNextComponentCandidates(
            m_Sigma, BackwardGSPA::ModelParam(1), m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::FGSPCA:
    {
        sparsePC.candidates = ForwardGSPCA::computeNextComponentCandidates(
            m_Sigma, ForwardGSPCA::ModelParam(1), m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::CUSTOM:
    {
        sparsePC.candidates = CustomSolver::computeNextComponentCandidates(
            m_Sigma, CustomSolver::ModelParam(1), m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::USERDYNAMICLIB:
    {
        auto& library = m_DynamicLibSolverLoaders.at(0);
        if(library)
        {
            auto computeSparseEigenVector =
                (ComputeSparseEigenVector)library->resolve("computeSparseEigenVector");
            if (computeSparseEigenVector)
            {
                sparsePC.candidates = DynamicLibSolver::computeNextComponentCandidates(
                    m_Sigma, DynamicLibSolver::ModelParam(computeSparseEigenVector), m_ValidatedComponents, m_ProgressBar);
            }
        }
        break;
    }
    default:
    {
        sparsePC.candidates = DCA::computeNextComponentCandidates(
            m_Sigma, DCA::ModelParam(1), m_ValidatedComponents, m_ProgressBar);
        break;
    }
    }

    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2000ms);
    }

    m_ProgressBar->setValue(m_N);

    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
    }

    m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ sparsePC.color + "; }");

    m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_SliderGroupBox);

    for(auto& [k, candidate]: sparsePC.candidates)
    {
        candidate.state = sparsepc::ComponentState::Unvalidated;
    }

    const int initialICandidate = m_N/2;

    const auto& component = sparsePC.candidates.at(initialICandidate);

    QString formattedValueStr = QString::number(m_CummulativeVarianceSparsePCs + component.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = formattedPCNumberStr;

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    sparsePC.pcColumn = ds->addColumn(m_N, curveName);

    ds->setAll(sparsePC.pcColumn, static_cast<double>(0));

    for (int i=0; i<m_N; ++i)
    {
        ds->inc(sparsePC.pcColumn, i, component.vector[i]);
    }

    const auto plotType = m_PlotTypeComboBox->currentData().value<PlotType>();
    switch (plotType)
    {
    case PlotType::FILLED:
    {
        auto* graph = new JKQTPFilledCurveXGraph(m_Plotter);

        auto col = QColor(sparsePC.color);
        graph->setLineStyle(Qt::DotLine);
        graph->setLineWidth(2);
        graph->setLineColor(col);

        graph->setFillMode(JKQTPFilledCurveXGraph::FillMode::SingleFilling);

        col.setAlphaF(0.25f);
        graph->setFillColor(col);
        graph->fillStyleBelow().setFillColor(col);
        graph->setBaseline(0.0);
        graph->setXColumn(m_ColumnX);
        graph->setYColumn(sparsePC.pcColumn);

        sparsePC.pcGraph = graph;
        break;
    }
    case PlotType::IMPULSES:
    {
        auto* graph = new JKQTPImpulsesVerticalGraph(m_Plotter);
        graph->setLineStyle(Qt::DotLine);
        graph->setLineWidth(3);
        graph->setDrawSymbols(true);
        graph->setSymbolType(JKQTPGraphSymbols::JKQTPCircle);
        auto col = QColor(sparsePC.color);
        graph->setColor(col);

        graph->setXColumn(m_ColumnX);
        graph->setYColumn(sparsePC.pcColumn);

        sparsePC.pcGraph = graph;
        break;
    }
    default:
    {
        break;
    }
    }

    sparsePC.pcGraph->setTitle(curveName + ": " + formattedValueStr);

    m_Plotter->addGraph(sparsePC.pcGraph);

    if( m_Slider->value() != initialICandidate )
    {
        m_Slider->setValue(initialICandidate);
    }
    else
    {
        updatePlot(initialICandidate);
    }
}

void AtelierWidget::onRemoveLastSparseComponentButton()
{
    if(m_SparsePCs.empty())
    {
        return;
    }

    m_Plotter->deleteGraph(m_SparsePCs.back().pcGraph, true);
    m_Plotter->getDatastore()->deleteColumn(m_SparsePCs.back().pcColumn, true);

    if(!m_ValidatedComponents.empty())
    {
        m_ValidatedComponents.back().get().state = sparsepc::ComponentState::Unvalidated;
        m_CummulativeVarianceSparsePCs -= m_ValidatedComponents.back().get().value;
        m_ValidatedComponents.pop_back();
    }

    if(!m_SparsePCs.empty())
    {
        m_SparsePCs.pop_back();
    }

    if(!m_SparsePCs.empty())
    {
        m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ m_SparsePCs.back().color + "; }");

        m_Slider->setValue(m_SparsePCs.back().iCandidate);
    }
    else
    {
        m_SliderGroupBox->setStyleSheet("");
        m_ProgressBarGroupBox->setStyleSheet("");
    }

    m_Plotter->redrawPlot();
}

void AtelierWidget::updatePlot(int value)
{
    if (m_SparsePCs.empty())
    {
        return;
    }

    auto& sparsePC = m_SparsePCs.back();

    sparsePC.iCandidate = value;

    const auto& candidate = sparsePC.candidates.at(sparsePC.iCandidate);

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    ds->setAll(sparsePC.pcColumn, static_cast<double>(0));

    for (int i=0; i<m_N; ++i)
    {
        ds->inc(sparsePC.pcColumn, i, candidate.vector[i]);
    }

    QString formattedValueStr = QString::number(m_CummulativeVarianceSparsePCs + candidate.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = formattedPCNumberStr;

    sparsePC.pcGraph->setTitle(curveName + ": " + formattedValueStr);

    m_Plotter->redrawPlot();
}

void AtelierWidget::onSelectionChanged(int index)
{// must replot all plots
    //m_PlotTypeComboBox->currentText();
    for(auto& spc: m_SparsePCs)
    {
        m_Plotter->deleteGraph(spc.pcGraph, true);
        m_Plotter->getDatastore()->deleteColumn(spc.pcColumn, true);
        spc.pcGraph = nullptr;
    }

    for(auto& spc: m_StandardPCs)
    {
        m_Plotter->deleteGraph(spc.pcGraph, true);
        m_Plotter->getDatastore()->deleteColumn(spc.pcColumn, true);
        spc.pcGraph = nullptr;
    }

    drawStandardPCs();
    drawSparsePCs();

    m_Plotter->redrawPlot();
}

void AtelierWidget::updateSliderTitle(int value)
{
    m_SliderGroupBox->setTitle(QString::number(value));
}

void AtelierWidget::updateProgressBarTitle(int value)
{
    const auto minValue = static_cast<double>(m_ProgressBar->minimum());
    const auto maxValue = static_cast<double>(m_ProgressBar->maximum());
    const auto percentage = static_cast<int>( (value - minValue) * 100.0 / (maxValue - minValue));
    m_ProgressBarGroupBox->setTitle(QString::number(percentage));
}

AtelierWidget::AtelierWidget(QWidget* parent)
    : QWidget(parent), m_Sigma{}, m_ValidatedComponents{},
    m_SparsePCs{}, m_StandardPCs{}, m_CummulativeVarianceStandardPCs{0.0},
    m_CummulativeVarianceSparsePCs{0}, m_Plotter{nullptr},
    m_SliderGroupBox{nullptr}, m_ProgressBarGroupBox{nullptr},
    m_Slider{nullptr}, m_ProgressBar{nullptr},
    m_SliderOrProgressBarWidgetStackedLayout{nullptr},
    m_MethodComboBox{nullptr}, m_PlotTypeComboBox{nullptr}, m_Colors{}
{
    qDebug() << "Loading plugins ...";

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

        qDebug() << "...Plugins loaded.";
    }
}

void AtelierWidget::createWidget()
{
    m_Colors.reserve(6);
    m_Colors.push_back("red");
    m_Colors.push_back("green");
    m_Colors.push_back("blue");
    m_Colors.push_back("magenta");
    m_Colors.push_back("yellow");
    m_Colors.push_back("cyan");

    auto* layout = new QGridLayout(this);

    auto* plotGroupbox = new QGroupBox(this);
    layout->addWidget(plotGroupbox, 0, 0);
    auto* plotGroupboxLayout = new QHBoxLayout(plotGroupbox);
    m_Plotter = new JKQTPlotter(this);
    m_Plotter->setWindowTitle("Plotter!!!!");
    m_Plotter->setPlotUpdateEnabled(true);

    //m_Plotter->getPlotter()->setUseAntiAliasingForGraphs(true); // nicer (but slower) plotting
    //m_Plotter->getPlotter()->setUseAntiAliasingForSystem(true); // nicer (but slower) plotting
    //m_Plotter->getPlotter()->setUseAntiAliasingForText(true); // nicer (but slower) text rendering

    JKQTPDatastore* datastore = m_Plotter->getDatastore();
    m_ColumnX = datastore->addLinearColumn(m_N, 0, m_N-1, "xi");
    plotGroupboxLayout->addWidget(m_Plotter);

    // slider
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

    m_ProgressBarGroupBox = new QGroupBox("0%", this);
    auto* progressBarGroupBoxLayout = new QHBoxLayout(m_ProgressBarGroupBox);

    m_ProgressBar = new QProgressBar(this);
    progressBarGroupBoxLayout->addWidget(m_ProgressBar);

    m_ProgressBar->setRange(0, m_N);
    m_ProgressBar->setValue(0);
    m_ProgressBar->setTextVisible(false);

    m_SliderOrProgressBarWidgetStackedLayout->addWidget(m_SliderGroupBox);
    m_SliderOrProgressBarWidgetStackedLayout->addWidget(m_ProgressBarGroupBox);
    m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_SliderGroupBox);

    auto* processingsGoupbox = new QGroupBox(this);
    layout->addWidget(processingsGoupbox, 2, 0);

    layout->setRowStretch(0, 12);
    layout->setRowStretch(1, 1);
    layout->setRowStretch(2, 1);

    auto* processingsGoupboxLayout = new QHBoxLayout(processingsGoupbox);

    auto* plotTypeGroupBox = new QGroupBox("Plot", this);
    auto* plotTypeGroupBoxLayout = new QHBoxLayout(plotTypeGroupBox);

    m_PlotTypeComboBox = new QComboBox(this);
    m_PlotTypeComboBox->addItem("Filled", QVariant::fromValue(PlotType::FILLED));
    m_PlotTypeComboBox->addItem("Impulses", QVariant::fromValue(PlotType::IMPULSES));

    plotTypeGroupBoxLayout->addWidget(m_PlotTypeComboBox);
    processingsGoupboxLayout->addWidget(plotTypeGroupBox);

    auto* methodGroupBox = new QGroupBox("Method", this);
    auto* methodGroupBoxLayout = new QHBoxLayout(methodGroupBox);

    m_MethodComboBox = new QComboBox(this);
    m_MethodComboBox->addItem("Dca", QVariant::fromValue(SpcaMethod::DCA));
    m_MethodComboBox->addItem("Forward Gspca", QVariant::fromValue(SpcaMethod::FGSPCA));
    m_MethodComboBox->addItem("Backward Gspca", QVariant::fromValue(SpcaMethod::BGSPCA));
    m_MethodComboBox->addItem("Custom", QVariant::fromValue(SpcaMethod::CUSTOM));
    if(!m_DynamicLibSolverLoaders.empty())
    {
        m_MethodComboBox->addItem(m_DynamicLibSolverNames.at(0), QVariant::fromValue(SpcaMethod::USERDYNAMICLIB));
    }

    methodGroupBoxLayout->addWidget(m_MethodComboBox);

    processingsGoupboxLayout->addWidget(methodGroupBox);

    auto* actionGroupBox = new QGroupBox("Sparse component", this);
    auto* actionGroupBoxLayout = new QHBoxLayout(actionGroupBox);

    processingsGoupboxLayout->addWidget(actionGroupBox);

    auto* addNewSparseComponentButton
        = new QPushButton("Add new", this);
    actionGroupBoxLayout->addWidget(addNewSparseComponentButton);

    auto* removeLastSparseComponentButton =
        new QPushButton("Remove last", this);
    actionGroupBoxLayout->addWidget(removeLastSparseComponentButton);

    QObject::connect(m_PlotTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                     this, &AtelierWidget::onSelectionChanged);
    QObject::connect(m_ProgressBar, &QProgressBar::valueChanged,
                     this, &AtelierWidget::updateProgressBarTitle);
    QObject::connect(m_Slider, qOverload<int>(&QSlider::valueChanged),
                     this, &AtelierWidget::updateSliderTitle);
    QObject::connect(m_Slider, qOverload<int>(&QSlider::valueChanged),
                     this, &AtelierWidget::updatePlot);
    QObject::connect(addNewSparseComponentButton, &QPushButton::clicked,
                     this, &AtelierWidget::onAddNewSparseComponent);
    QObject::connect(removeLastSparseComponentButton, &QPushButton::clicked,
                     this, &AtelierWidget::onRemoveLastSparseComponentButton);

    processingsGoupboxLayout->addStretch(1);
}

void AtelierWidget::init(const QString& fileName, bool newProject)
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

        createWidget();
        computeStandardPCs();
        drawStandardPCs();
    }
    else
    {
        loadProject(fileName);

        createWidget();
        drawStandardPCs();
        drawSparsePCs();
        if(!m_SparsePCs.empty())
        {
            m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ m_SparsePCs.back().color + "; }");
        }
    }

    zoomToFit();
}

// write operator
QDataStream &operator<<(QDataStream &out, const MyClass &user)
{
    out << static_cast<qint32>(user.iCandidate);
    out << user.color;
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
    in >> user.color;
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

void AtelierWidget::saveProject(const QString& filename)
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

void AtelierWidget::loadProject(const QString& fileName)
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

void AtelierWidget::zoomToFit()
{
    m_Plotter->setAbsoluteX(static_cast<double>(0), static_cast<double>(m_N-1));
    m_Plotter->setAbsoluteY(static_cast<double>(-1), static_cast<double>(1));

    m_Plotter->zoomToFit(true, false);

    m_Plotter->resize(400,300);
}
