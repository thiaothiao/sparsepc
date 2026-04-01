#include "AtelierWidget.h"

#include <thread>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>

#include <QColor>
#include <QString>

#include "sparsepc/core.hpp"
#include "simu.hpp"
#include <Eigen/Dense>

namespace
{
    enum class SpcaMethod : std::uint8_t
    {
        DCA = 0U,
        FGSPCA,  //ForwardGSPCA
        BGSPCA   // BackwardGSPA
    };

    //std::cout << "\nStarting backward run.\n";
    using BackwardGSPA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::BackwardGspcaModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;

    using ForwardGSPCA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::ForwardGspcaModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;

    using DCA = sparsepc::linearmodel::SparsePC<
        sparsepc::linearmodel::DcaModel<double, sparsepc::EigenSolver<double>, QProgressBar>>;
}

Q_DECLARE_METATYPE(SpcaMethod)

void AtelierWidget::drawStandardPCs()
{
    using Matrix = sparsepc::Matrix<double>;
    using Vector = sparsepc::Vector<double>;
    using Index = sparsepc::Index;

    const auto n = m_Sigma.cols();

    const Index k0 = n;
    const Index k1 = n;
    const Index k2 = n;

    const DCA::Param param{ {k0, k1, k2} };

    const auto sparseEigenElements = DCA{ param }.run(m_Sigma);

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    m_Colors.reserve(sparseEigenElements.size());
    m_Colors.push_back(QColorConstants::Svg::orange);
    m_Colors.push_back(QColorConstants::Svg::limegreen);
    m_Colors.push_back(QColorConstants::Svg::plum);
    m_Colors.push_back(QColorConstants::Svg::cyan);
    m_Colors.push_back(QColorConstants::Svg::magenta);

    m_PCGraphs.reserve(n);
    for (int j=0; j<sparseEigenElements.size(); ++j)
    {
        const auto& element = sparseEigenElements[j];

        QString formattedValueStr = QString::number(element.value, 'f', 2);
        QString formattedPCNumberStr = QString::number(j);
        QString curveName = QString("pc") + formattedPCNumberStr;
        size_t column = ds->addColumn(n, curveName);

        ds->setAll(column, static_cast<double>(0));

        for (int i=0; i<n; ++i)
        {
            ds->inc(column, i, element.vector[i]);
        }

        //JKQTPXYScatterGraph
        //JKQTPXYLineGraph
        m_PCGraphs.push_back(new JKQTPXYLineGraph(m_Plotter));
        auto& graph = m_PCGraphs.back();

        graph->setTitle(curveName + ": " + formattedValueStr);

        QColor col = m_Colors[j];
        graph->setColor(col);
        //col.setAlphaF(0.125f);
        //graph->setFillColor(col);

        graph->setLineStyle(Qt::DotLine);
        graph->setLineWidth(1);

        graph->setXColumn(m_ColumnX);
        graph->setYColumn(column);

        m_Plotter->addGraph(graph);
    }

    // 5. set axis labels
    //m_Plotter->getXAxis()->setAxisLabel("features");
    //m_Plotter->getYAxis()->setAxisLabel("magnitudes");

    // 4. set the maximum size of the plot to 0..100% and 0..256
    m_Plotter->setAbsoluteX(static_cast<double>(0), static_cast<double>(n-1));
    m_Plotter->setAbsoluteY(static_cast<double>(-1), static_cast<double>(1));

    // ... and scale plot automatically
    m_Plotter->zoomToFit(true, false);

    // 5. show plotter and make it a decent size
    m_Plotter->resize(400,300);
    //plot.show();
}

void AtelierWidget::onAddNewSparseComponent()
{
    if(!m_MyClasses.empty())
    {// validate current selected sparse component
        auto& candidates = m_MyClasses.back().candidates;
        const auto iCandidate = m_MyClasses.back().iCandidate;
        candidates[iCandidate].state = sparsepc::ComponentState::Validated;
        m_ValidatedComponents.push_back(candidates[iCandidate]);// TODO use std::move
    }

    m_MyClasses.push_back({});// Why not emplace_back

    auto& myClass = m_MyClasses.back();

    myClass.iCandidate = -1;

    myClass.color = m_Colors[m_ValidatedComponents.size()];
    const QString colorString =
        QString("rgb(%1, %2, %3)").arg(myClass.color.red()).arg(myClass.color.green()).arg(myClass.color.blue());

    m_ProgressBarGroupBox->setStyleSheet("QGroupBox::title { color: "+ colorString + "; }");

    const auto n = m_Sigma.cols();

    m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_ProgressBarGroupBox);// why not set current index

    m_ProgressBar->setValue(0);

    const auto method = m_MethodComboBox->currentData().value<SpcaMethod>();
    switch (method)
    {
    case SpcaMethod::DCA:
    {
        const DCA::Param param{ {1} };
        myClass.candidates = DCA::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::BGSPCA:
    {
        const BackwardGSPA::Param param{ {1} };
        myClass.candidates = BackwardGSPA::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::FGSPCA:
    {
        const ForwardGSPCA::Param param{ {1} };
        myClass.candidates = ForwardGSPCA::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents, m_ProgressBar);
        break;
    }
    default:
    {
        const DCA::Param param{ {1} };
        myClass.candidates = DCA::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents, m_ProgressBar);
        break;
    }
    }

    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2000ms);
    }

    m_ProgressBar->setValue(n);

    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
    }

    m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ colorString + "; }");

    m_SliderOrProgressBarWidgetStackedLayout->setCurrentWidget(m_SliderGroupBox);// why not set current index

    for(auto& [k, candidate]: myClass.candidates)
    {
        candidate.state = sparsepc::ComponentState::Unvalidated;
    }

    const int initialICandidate = n/2;

    const auto& element = myClass.candidates.at(initialICandidate);

    QString formattedValueStr = QString::number(element.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = QString("spc") + formattedPCNumberStr;

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    myClass.sPCColumn = ds->addColumn(n, curveName);

    ds->setAll(myClass.sPCColumn, static_cast<double>(0));

    for (int i=0; i<n; ++i)
    {
        ds->inc(myClass.sPCColumn, i, element.vector[i]);
    }

    myClass.sPCGraph = new JKQTPXYLineGraph(m_Plotter);

    myClass.sPCGraph->setTitle(curveName + ": " + formattedValueStr);

    myClass.sPCGraph->setColor(myClass.color);
    //myClass.color.setAlphaF(0.125f);
    //myClass.sPCGraph->setFillColor(myClass.color);

    myClass.sPCGraph->setLineWidth(2);

    myClass.sPCGraph->setXColumn(m_ColumnX);
    myClass.sPCGraph->setYColumn(myClass.sPCColumn);

    m_Plotter->addGraph(myClass.sPCGraph);

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
    if(m_MyClasses.empty())
    {
        return;
    }

    m_Plotter->deleteGraph(m_MyClasses.back().sPCGraph, true);
    m_Plotter->getDatastore()->deleteColumn(m_MyClasses.back().sPCColumn, true);

    if(!m_ValidatedComponents.empty())
    {
        m_ValidatedComponents.pop_back();
    }

    if(!m_MyClasses.empty())
    {
        m_MyClasses.pop_back();
    }

    if(!m_MyClasses.empty())
    {
        const QString colorString =
            QString("rgb(%1, %2, %3)").arg(m_MyClasses.back()
            .color.red()).arg(m_MyClasses.back().color.green())
            .arg(m_MyClasses.back().color.blue());

        m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ colorString + "; }");

        m_Slider->setValue(m_MyClasses.back().iCandidate);
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
    if (m_MyClasses.empty())
    {
        return;
    }

    auto& myClass = m_MyClasses.back();

    myClass.iCandidate = value;

    const auto& candidate = myClass.candidates.at(myClass.iCandidate);

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    ds->setAll(myClass.sPCColumn, static_cast<double>(0));

    const int n = candidate.vector.size();
    for (int i=0; i<n; ++i)
    {
        ds->inc(myClass.sPCColumn, i, candidate.vector[i]);
    }

    QString formattedValueStr = QString::number(candidate.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = QString("spc") + formattedPCNumberStr;

    myClass.sPCGraph->setTitle(curveName + ": " + formattedValueStr);

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
    m_MyClasses{}, m_PCGraphs{}, m_Plotter{nullptr},
    m_SliderGroupBox{nullptr}, m_ProgressBarGroupBox{nullptr},
    m_Slider{nullptr}, m_ProgressBar{nullptr},
    m_SliderOrProgressBarWidgetStackedLayout{nullptr},
    m_MethodComboBox{nullptr}, m_Colors{}
{
    using Matrix = sparsepc::Matrix<double>;
    using Vector = sparsepc::Vector<double>;
    using Index = sparsepc::Index;

    m_Sigma = sparsepc::linearmodel::pitprops<double>();
    const auto n = m_Sigma.cols();

    m_ValidatedComponents.reserve(n);
    m_MyClasses.reserve(n);

    auto* layout = new QGridLayout(this);

    // plot
    auto* plotGroupbox = new QGroupBox(this);
    layout->addWidget(plotGroupbox, 0, 0);
    auto* plotGroupboxLayout = new QHBoxLayout(plotGroupbox);
    m_Plotter = new JKQTPlotter(this);
    m_Plotter->setWindowTitle("Plotter!!!!");
    m_Plotter->setPlotUpdateEnabled(true);
    JKQTPDatastore* datastore = m_Plotter->getDatastore();
    m_ColumnX = datastore->addLinearColumn(n, 0, n-1, "xi");
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

    m_SliderOrProgressBarWidgetStackedLayout->addWidget(m_SliderGroupBox);

    m_Slider->setRange(1, n);
    m_Slider->setSingleStep(1);

    m_ProgressBarGroupBox = new QGroupBox("0%", this);
    auto* progressBarGroupBoxLayout = new QHBoxLayout(m_ProgressBarGroupBox);

    m_SliderOrProgressBarWidgetStackedLayout->addWidget(m_ProgressBarGroupBox);

    m_ProgressBar = new QProgressBar(this);
    progressBarGroupBoxLayout->addWidget(m_ProgressBar);

    m_ProgressBar->setRange(0, n);
    m_ProgressBar->setValue(0);
    m_ProgressBar->setTextVisible(false);

    // processings
    auto* processingsGoupbox = new QGroupBox(this);
    layout->addWidget(processingsGoupbox, 2, 0);

    layout->setRowStretch(0, 12);
    layout->setRowStretch(1, 1);
    layout->setRowStretch(2, 1);

    auto* processingsGoupboxLayout = new QHBoxLayout(processingsGoupbox);

    auto* methodGroupBox = new QGroupBox("Method", this);
    auto* methodGroupBoxLayout = new QHBoxLayout(methodGroupBox);

    m_MethodComboBox = new QComboBox(this);
    m_MethodComboBox->addItem("Dca", QVariant::fromValue(SpcaMethod::DCA));
    m_MethodComboBox->addItem("Forward Gspca", QVariant::fromValue(SpcaMethod::FGSPCA));
    m_MethodComboBox->addItem("Backward Gspca", QVariant::fromValue(SpcaMethod::BGSPCA));

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

    // connect

    QObject::connect(m_ProgressBar, &QProgressBar::valueChanged,
        this, &AtelierWidget::updateProgressBarTitle);
    QObject::connect(m_Slider, &QSlider::valueChanged,
        this, &AtelierWidget::updateSliderTitle);
    QObject::connect(m_Slider, &QSlider::valueChanged,
        this, &AtelierWidget::updatePlot);
    QObject::connect(addNewSparseComponentButton, &QPushButton::clicked,
        this, &AtelierWidget::onAddNewSparseComponent);
    QObject::connect(removeLastSparseComponentButton, &QPushButton::clicked,
        this, &AtelierWidget::onRemoveLastSparseComponentButton);

    processingsGoupboxLayout->addStretch(1);

    drawStandardPCs();
}
