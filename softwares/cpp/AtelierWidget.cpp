#include "AtelierWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QFrame>

#include <QColor>//Constants>

#include <QString>

#include <QApplication>

#include "sparsepc/version.hpp"
#include "sparsepc/core.hpp"
#include "simu.hpp"
#include <Eigen/Dense>

void AtelierWidget::drawPCs()
{
    constexpr std::string_view version = SPARSEPC_MACRO_STRINGIFY(SPARSEPC_VERSION);

    using Scalar = double;
    using Matrix = sparsepc::Matrix<Scalar>;
    using Vector = sparsepc::Vector<Scalar>;
    using Index = sparsepc::Index;

    const auto n = m_Sigma.cols();

    const Index k0 = 13;
    const Index k1 = 13;
    const Index k2 = 13;

    //std::cout << "\nStarting backward run.\n";
    using BackwardGspca = sparsepc::linearmodel::BackwardGspca<Scalar>;

    const BackwardGspca::Param param{ {k0, k1, k2} };

    const auto sparseEigenElements = BackwardGspca{ param }.run(m_Sigma);

    //std::cout << sparsepc::toMatrix<Scalar>(sparseEigenElements) << "\n";

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    m_Colors.reserve(sparseEigenElements.size());
    m_Colors.push_back(QColorConstants::Svg::blue);//QColor("blue"));
    m_Colors.push_back(QColorConstants::Svg::orange);//QColor("red"));
    m_Colors.push_back(QColorConstants::Svg::limegreen);//QColor("green"));;
    //'blue', 'orange', 'limegreen'

    m_PCGraphs.reserve(n);
    m_SPCGraphs.reserve(n);
    for (int j=0; j<sparseEigenElements.size(); ++j)
    {
        const auto& element = sparseEigenElements[j];

        QString formattedValueStr = QString::number(element.value, 'f', 2);
        QString formattedPCNumberStr = QString::number(j+1);
        QString curveName = QString("PC") + formattedPCNumberStr;
        size_t column = ds->addColumn(n, curveName);

        ds->setAll(column, static_cast<Scalar>(0));

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

        //graph->setLineStyle(Qt::DotLine); // Sets to dotted
        graph->setLineWidth(2);

        graph->setXColumn(m_ColumnX);
        graph->setYColumn(column);

        m_Plotter->addGraph(graph);
    }

    // 5. set axis labels
    //m_Plotter->getXAxis()->setAxisLabel("features");
    //m_Plotter->getYAxis()->setAxisLabel("magnitudes");

    // 4. set the maximum size of the plot to 0..100% and 0..256
    m_Plotter->setAbsoluteX(static_cast<Scalar>(0), static_cast<Scalar>(n-1));
    m_Plotter->setAbsoluteY(static_cast<Scalar>(-1), static_cast<Scalar>(1));

    // ... and scale plot automatically
    m_Plotter->zoomToFit(true, false);

    // 5. show plotter and make it a decent size
    m_Plotter->resize(400,300);
    //plot.show();
}

void AtelierWidget::onAddNewSparseComponent()
{
    if(!m_Candidates.empty())
    {// validate current delcted sparse component
        m_ValidatedComponents.push_back(m_Candidates[m_ICandidate]);// TODO use std::move
    }

    using Scalar = double;
    using BackwardGspca = sparsepc::linearmodel::BackwardGspca<Scalar>;
    using Component = sparsepc::Component<Scalar>;
    using ComponentsContainer = std::vector<Component>;

    const BackwardGspca::Param param{ {1} };

    m_Candidates = BackwardGspca::computeNextComponentCandidates(
            m_Sigma, param.modelParams[0], m_ValidatedComponents);

    for(auto& candidate: m_Candidates)
    {
        candidate.state = sparsepc::ComponentState::Unvalidated;
    }

    const auto n = m_Sigma.cols();

    const int initialICandidate = n/2;

    const auto& element = m_Candidates[initialICandidate];

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    m_SPCGraphs.push_back(new JKQTPXYLineGraph(m_Plotter));
    auto& graph = m_SPCGraphs.back();

    QString formattedValueStr = QString::number(element.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size()+1);
    QString curveName = QString("SPC") + formattedPCNumberStr;
    size_t column = ds->addColumn(n, curveName);
    m_SPCColumns.push_back(column);

    ds->setAll(column, static_cast<Scalar>(0));

    for (int i=0; i<n; ++i)
    {
        ds->inc(column, i, element.vector[i]);
    }

    graph->setTitle(curveName + ": " + formattedValueStr);

    QColor col = m_Colors[m_ValidatedComponents.size()];
    graph->setColor(col);
    //col.setAlphaF(0.125f);
    //graph->setFillColor(col);

    graph->setLineStyle(Qt::DotLine); // Sets to dotted
    graph->setLineWidth(2);

    graph->setXColumn(m_ColumnX);
    graph->setYColumn(column);

    m_Plotter->addGraph(graph);

    //m_Plotter->redrawPlot();
    //updatePlot(n-1);
    m_SparsityLevelSlider->setValue(initialICandidate);
}

void AtelierWidget::onClearSparseCandidates()
{
    if(m_Candidates.empty())
    {
        return;
    }

    m_Plotter->deleteGraph(m_SPCGraphs.back());
    m_SPCGraphs.pop_back();

    JKQTPDatastore* ds = m_Plotter->getDatastore();
    ds->deleteColumn(m_SPCColumns.back(), false);

    m_SPCColumns.pop_back();

    m_Candidates.clear();

    m_Plotter->redrawPlot();
}

void AtelierWidget::updatePlot(int iCandidate)
{
    using Scalar = double;

    if (m_Candidates.empty())
    {
        return;
    }

    const auto n = m_Sigma.cols();

    m_ICandidate = iCandidate;

    const auto& element = m_Candidates[m_ICandidate];

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    if(!m_SPCGraphs.empty())
    {// just update plot data
        size_t column = m_SPCColumns.back();
        ds->setAll(column, static_cast<Scalar>(0));

        for (int i=0; i<n; ++i)
        {
            ds->inc(column, i, element.vector[i]);
        }

        auto& graph = m_SPCGraphs.back();

        QString formattedValueStr = QString::number(element.value, 'f', 2);
        QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size()+1);
        QString curveName = QString("SPC") + formattedPCNumberStr;

        graph->setTitle(curveName + ": " + formattedValueStr);

        m_Plotter->redrawPlot();
    }
}

AtelierWidget::AtelierWidget(QWidget* parent)
    : QWidget(parent)
{
    using Scalar = double;
    using Matrix = sparsepc::Matrix<Scalar>;
    using Vector = sparsepc::Vector<Scalar>;
    using Index = sparsepc::Index;

    m_Sigma = sparsepc::linearmodel::pitprops<Scalar>();
    const auto n = m_Sigma.cols();

    m_ValidatedComponents.reserve(n);

    auto* layout = new QVBoxLayout(this);
    //setLayout(layout);

    /************************/
    auto* plotGroupbox = new QGroupBox(this);
    layout->addWidget(plotGroupbox);

    auto* plotGroupboxLayout = new QHBoxLayout(plotGroupbox);
    /**************************/

    m_Plotter = new JKQTPlotter(this);
    m_Plotter->setWindowTitle("Plotter!!!!");
    m_Plotter->setPlotUpdateEnabled(true);

    JKQTPDatastore* datastore = m_Plotter->getDatastore();

    m_ColumnX = datastore->addLinearColumn(n, 0, n-1, "xi");

    plotGroupboxLayout->addWidget(m_Plotter);

    // slider
    auto* sliderGoupbox = new QGroupBox(this);
    layout->addWidget(sliderGoupbox);

    auto* sliderGoupboxLayout = new QHBoxLayout(sliderGoupbox);

    m_SparsityLevelSlider = new QSlider(Qt::Orientation::Horizontal, this);
    sliderGoupboxLayout->addWidget(m_SparsityLevelSlider);

    const auto mini = 1;
    const auto maxi = n-1;
    m_SparsityLevelSlider->setRange(mini, maxi);

    //sliderGoupbox->setLayout(sliderGoupboxLayout);

    QObject::connect(m_SparsityLevelSlider, &QSlider::valueChanged,
                     this, &AtelierWidget::updatePlot);

    // processings
    auto* processingsGoupbox = new QGroupBox(this);
    layout->addWidget(processingsGoupbox);

    auto* processingsGoupboxLayout = new QHBoxLayout(processingsGoupbox);

    auto* addNewSparseComponentButton
        = new QPushButton("Add new\n sparse component", this);

    processingsGoupboxLayout->addWidget(addNewSparseComponentButton);

    QObject::connect(addNewSparseComponentButton, &QPushButton::clicked,
                     this, &AtelierWidget::onAddNewSparseComponent);

    auto* clearSparseCandidatesButton = new QPushButton("Remove last\n sparse component", this);
    processingsGoupboxLayout->addWidget(clearSparseCandidatesButton);

    processingsGoupboxLayout->addStretch(1);

    //processingsGoupbox->setLayout(processingsGoupboxLayout);

    //QObject::connect(clearSparseCandidatesButton, &QPushButton::clicked,
    //                 this, &AtelierWidget::onClearSparseCandidates);

    //drawPCs<JKQTPFilledCurveXGraph>(plotX);
    drawPCs();
}


