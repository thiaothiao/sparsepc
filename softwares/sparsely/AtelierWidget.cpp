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
#include <QPen>

#include <QColor>
#include <QString>

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

    auto components = DCA{ param }.run(m_Sigma);

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    m_Colors.reserve(components.size());
    m_Colors.push_back(QColor("red"));
    m_Colors.push_back(QColor("green"));
    m_Colors.push_back(QColor("blue"));
    m_Colors.push_back(QColor("magenta"));
    m_Colors.push_back(QColor("yellow"));
    m_Colors.push_back(QColor("cyan"));

    for (int j=0; j<components.size(); ++j)
    {
        m_StandardPCs.push_back({});// Why not emplace_back

        auto& standardPC = m_StandardPCs.back();

        auto& component = standardPC.candidates.emplace(n, std::move(components[j])).first->second;
        component.state = sparsepc::ComponentState::Validated;

        standardPC.iCandidate = n;

        standardPC.color = m_Colors[j];

        QString formattedValueStr = QString::number(component.value, 'f', 2);
        QString formattedPCNumberStr = QString::number(j);
        QString curveName = formattedPCNumberStr;
        size_t column = ds->addColumn(n, curveName);

        ds->setAll(column, static_cast<double>(0));

        for (int i=0; i<n; ++i)
        {
            ds->inc(column, i, component.vector[i]);
        }

        standardPC.pcGraph = new JKQTPFilledCurveXGraph(m_Plotter);

        auto col = standardPC.color;
        standardPC.pcGraph->setLineStyle(Qt::SolidLine);
        standardPC.pcGraph->setLineWidth(1);
        standardPC.pcGraph->setLineColor(col);

        standardPC.pcGraph->setFillMode(JKQTPFilledCurveXGraph::FillMode::SingleFilling);

        col.setAlphaF(0.125f);
        standardPC.pcGraph->setFillColor(col);
        standardPC.pcGraph->fillStyleBelow().setFillColor(col);
        standardPC.pcGraph->setBaseline(0.0);

        standardPC.pcGraph->setXColumn(m_ColumnX);
        standardPC.pcGraph->setYColumn(column);

        standardPC.pcGraph->setTitle(curveName + ": " + formattedValueStr);

        m_Plotter->addGraph(standardPC.pcGraph);
    }

    // set the maximum size of the plot
    m_Plotter->setAbsoluteX(static_cast<double>(0), static_cast<double>(n-1));
    m_Plotter->setAbsoluteY(static_cast<double>(-1), static_cast<double>(1));

    //scale plot automatically
    m_Plotter->zoomToFit(true, false);

    m_Plotter->resize(400,300);
}

void AtelierWidget::onAddNewSparseComponent()
{
    if(!m_SparsePCs.empty())
    {// validate current selected sparse component
        auto& candidates = m_SparsePCs.back().candidates;
        const auto iCandidate = m_SparsePCs.back().iCandidate;
        candidates[iCandidate].state = sparsepc::ComponentState::Validated;
        m_ValidatedComponents.push_back(candidates[iCandidate]);// TODO use std::move
    }

    m_SparsePCs.push_back({});// Why not emplace_back

    auto& sparsePC = m_SparsePCs.back();

    sparsePC.iCandidate = -1;

    sparsePC.color = m_Colors[m_ValidatedComponents.size()];
    const QString colorString =
        QString("rgb(%1, %2, %3)").arg(sparsePC.color.red()).arg(sparsePC.color.green()).arg(sparsePC.color.blue());

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
        sparsePC.candidates = DCA::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::BGSPCA:
    {
        const BackwardGSPA::Param param{ {1} };
        sparsePC.candidates = BackwardGSPA::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents, m_ProgressBar);
        break;
    }
    case SpcaMethod::FGSPCA:
    {
        const ForwardGSPCA::Param param{ {1} };
        sparsePC.candidates = ForwardGSPCA::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents, m_ProgressBar);
        break;
    }
    default:
    {
        const DCA::Param param{ {1} };
        sparsePC.candidates = DCA::computeNextComponentCandidates(
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

    for(auto& [k, candidate]: sparsePC.candidates)
    {
        candidate.state = sparsepc::ComponentState::Unvalidated;
    }

    const int initialICandidate = n/2;

    const auto& component = sparsePC.candidates.at(initialICandidate);

    QString formattedValueStr = QString::number(component.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = formattedPCNumberStr;

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    sparsePC.pcColumn = ds->addColumn(n, curveName);

    ds->setAll(sparsePC.pcColumn, static_cast<double>(0));

    for (int i=0; i<n; ++i)
    {
        ds->inc(sparsePC.pcColumn, i, component.vector[i]);
    }

    sparsePC.pcGraph = new JKQTPFilledCurveXGraph(m_Plotter);

    auto col = sparsePC.color;
    sparsePC.pcGraph->setLineStyle(Qt::DotLine);
    sparsePC.pcGraph->setLineWidth(2);
    sparsePC.pcGraph->setLineColor(col);

    sparsePC.pcGraph->setFillMode(JKQTPFilledCurveXGraph::FillMode::SingleFilling);

    col.setAlphaF(0.25f);
    sparsePC.pcGraph->setFillColor(col);
    sparsePC.pcGraph->fillStyleBelow().setFillColor(col);
    sparsePC.pcGraph->setBaseline(0.0);

    sparsePC.pcGraph->setXColumn(m_ColumnX);
    sparsePC.pcGraph->setYColumn(sparsePC.pcColumn);

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
        m_ValidatedComponents.pop_back();
    }

    if(!m_SparsePCs.empty())
    {
        m_SparsePCs.pop_back();
    }

    if(!m_SparsePCs.empty())
    {
        const QString colorString =
            QString("rgb(%1, %2, %3)").arg(m_SparsePCs.back()
            .color.red()).arg(m_SparsePCs.back().color.green())
            .arg(m_SparsePCs.back().color.blue());

        m_SliderGroupBox->setStyleSheet("QGroupBox::title { color: "+ colorString + "; }");

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

    const int n = candidate.vector.size();
    for (int i=0; i<n; ++i)
    {
        ds->inc(sparsePC.pcColumn, i, candidate.vector[i]);
    }

    QString formattedValueStr = QString::number(candidate.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = formattedPCNumberStr;

    sparsePC.pcGraph->setTitle(curveName + ": " + formattedValueStr);

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

void AtelierWidget::save(bool checked)
{

}

AtelierWidget::AtelierWidget(QWidget* parent)
    : QWidget(parent), m_Sigma{}, m_ValidatedComponents{},
    m_SparsePCs{}, m_StandardPCs{}, m_Plotter{nullptr},
    m_SliderGroupBox{nullptr}, m_ProgressBarGroupBox{nullptr},
    m_Slider{nullptr}, m_ProgressBar{nullptr},
    m_SliderOrProgressBarWidgetStackedLayout{nullptr},
    m_MethodComboBox{nullptr}, m_Colors{}
{
}


void AtelierWidget::init(sparsepc::Matrix<double>&& sigma)
{
    m_Sigma = std::move(sigma);

    const auto n = m_Sigma.cols();

    m_ValidatedComponents.reserve(n);
    m_SparsePCs.reserve(n);
    m_StandardPCs.reserve(n);

    auto* layout = new QGridLayout(this);

    // plot
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
