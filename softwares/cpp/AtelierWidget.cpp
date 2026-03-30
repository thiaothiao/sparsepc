#include "AtelierWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

#include <QColor>
#include <QString>

#include "sparsepc/core.hpp"
#include "simu.hpp"
#include <Eigen/Dense>

void AtelierWidget::drawStandardPCs()
{
    using Matrix = sparsepc::Matrix<double>;
    using Vector = sparsepc::Vector<double>;
    using Index = sparsepc::Index;

    const auto n = m_Sigma.cols();

    const Index k0 = 13;
    const Index k1 = 13;
    const Index k2 = 13;

    //std::cout << "\nStarting backward run.\n";
    using BackwardGspca = sparsepc::linearmodel::BackwardGspca<double>;

    const BackwardGspca::Param param{ {k0, k1, k2} };

    const auto sparseEigenElements = BackwardGspca{ param }.run(m_Sigma);

    //std::cout << sparsepc::toMatrix<double>(sparseEigenElements) << "\n";

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    m_Colors.reserve(sparseEigenElements.size());
    m_Colors.push_back(QColorConstants::Svg::blue);
    m_Colors.push_back(QColorConstants::Svg::orange);
    m_Colors.push_back(QColorConstants::Svg::limegreen);
    m_Colors.push_back(QColorConstants::Svg::plum);
    m_Colors.push_back(QColorConstants::Svg::cyan);

    const QString colorString =
        QString("rgb(%1, %2, %3)").arg(m_Colors[0].red()).arg(m_Colors[0].green()).arg(m_Colors[0].blue());

    m_SparsityLevelSlider->setStyleSheet(
        "QSlider::handle:horizontal { background: " + colorString + "; }");

    m_PCGraphs.reserve(n);
    for (int j=0; j<sparseEigenElements.size(); ++j)
    {
        const auto& element = sparseEigenElements[j];

        QString formattedValueStr = QString::number(element.value, 'f', 2);
        QString formattedPCNumberStr = QString::number(j);
        QString curveName = QString("PC") + formattedPCNumberStr;
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

    using BackwardGspca = sparsepc::linearmodel::BackwardGspca<double>;
    using Component = sparsepc::Component<double>;
    using ComponentsContainer = std::vector<Component>;

    const BackwardGspca::Param param{ {1} };

    m_MyClasses.push_back({});// Why not emplace_back

    auto& myClass = m_MyClasses.back();

    myClass.iCandidate = -1;

    myClass.candidates = BackwardGspca::computeNextComponentCandidates(
        m_Sigma, param.modelParams[0], m_ValidatedComponents);

    for(auto& candidate: myClass.candidates)
    {
        candidate.state = sparsepc::ComponentState::Unvalidated;
    }

    const auto n = m_Sigma.cols();

    const int initialICandidate = n/2;

    const auto& element = myClass.candidates[initialICandidate-1];

    QString formattedValueStr = QString::number(element.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = QString("SPC") + formattedPCNumberStr;

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    myClass.sPCColumn = ds->addColumn(n, curveName);

    ds->setAll(myClass.sPCColumn, static_cast<double>(0));

    for (int i=0; i<n; ++i)
    {
        ds->inc(myClass.sPCColumn, i, element.vector[i]);
    }

    myClass.sPCGraph = new JKQTPXYLineGraph(m_Plotter);

    myClass.sPCGraph->setTitle(curveName + ": " + formattedValueStr);

    myClass.color = m_Colors[m_ValidatedComponents.size()];
    myClass.sPCGraph->setColor(myClass.color);
    //myClass.color.setAlphaF(0.125f);
    //myClass.sPCGraph->setFillColor(myClass.color);

    const QString colorString =
        QString("rgb(%1, %2, %3)").arg(myClass.color.red()).arg(myClass.color.green()).arg(myClass.color.blue());

    m_SparsityLevelSlider->setStyleSheet(
        "QSlider::handle:horizontal { background: " + colorString + "; }");

    //myClass.sPCGraph->setLineStyle(Qt::DotLine);
    myClass.sPCGraph->setLineWidth(2);

    myClass.sPCGraph->setXColumn(m_ColumnX);
    myClass.sPCGraph->setYColumn(myClass.sPCColumn);

    m_Plotter->addGraph(myClass.sPCGraph);

    if( m_SparsityLevelSlider->value() != initialICandidate+1 )
    {
        m_SparsityLevelSlider->setValue(initialICandidate+1);
    }
    else
    {
        updatePlot(initialICandidate+1);
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

        m_SparsityLevelSlider->setStyleSheet(
            "QSlider::handle:horizontal { background: " + colorString + "; }");

        m_SparsityLevelSlider->setValue(m_MyClasses.back().iCandidate + 1);
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

    myClass.iCandidate = value-1;

    const auto& candidate = myClass.candidates[myClass.iCandidate];

    JKQTPDatastore* ds = m_Plotter->getDatastore();

    ds->setAll(myClass.sPCColumn, static_cast<double>(0));

    const int n = candidate.vector.size();
    for (int i=0; i<n; ++i)
    {
        ds->inc(myClass.sPCColumn, i, candidate.vector[i]);
    }

    QString formattedValueStr = QString::number(candidate.value, 'f', 2);
    QString formattedPCNumberStr = QString::number(m_ValidatedComponents.size());
    QString curveName = QString("SPC") + formattedPCNumberStr;

    myClass.sPCGraph->setTitle(curveName + ": " + formattedValueStr);

    m_Plotter->redrawPlot();
}

AtelierWidget::AtelierWidget(QWidget* parent)
    : QWidget(parent), m_Sigma{}, m_ValidatedComponents{},
    m_MyClasses{}, m_PCGraphs{}, m_Plotter{nullptr},
    m_SparsityLevelSlider{nullptr}, m_Colors{}
{
    using Matrix = sparsepc::Matrix<double>;
    using Vector = sparsepc::Vector<double>;
    using Index = sparsepc::Index;

    m_Sigma = sparsepc::linearmodel::pitprops<double>();
    const auto n = m_Sigma.cols();

    m_ValidatedComponents.reserve(n);
    m_MyClasses.reserve(n);

    auto* layout = new QVBoxLayout(this);

    // plot
    auto* plotGroupbox = new QGroupBox(this);
    layout->addWidget(plotGroupbox);
    auto* plotGroupboxLayout = new QHBoxLayout(plotGroupbox);
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
    auto* sliderlabel = new QLabel("1", this);
    m_SparsityLevelSlider = new QSlider(Qt::Orientation::Horizontal, this);
    sliderGoupboxLayout->addWidget(sliderlabel);
    sliderGoupboxLayout->addWidget(m_SparsityLevelSlider);

    m_SparsityLevelSlider->setRange(1, n);
    m_SparsityLevelSlider->setSingleStep(1);

    // processings
    auto* processingsGoupbox = new QGroupBox(this);
    layout->addWidget(processingsGoupbox);

    auto* processingsGoupboxLayout = new QHBoxLayout(processingsGoupbox);

    auto* addNewSparseComponentButton
        = new QPushButton("Add new\n sparse component", this);
    processingsGoupboxLayout->addWidget(addNewSparseComponentButton);

    auto* removeLastSparseComponentButton =
        new QPushButton("Remove last\n sparse component", this);
    processingsGoupboxLayout->addWidget(removeLastSparseComponentButton);

    // connect
    QObject::connect(m_SparsityLevelSlider, &QSlider::valueChanged,
        this, [sliderlabel](int value){sliderlabel->setText(QString::number(value));});

    QObject::connect(m_SparsityLevelSlider, &QSlider::valueChanged,
        this, &AtelierWidget::updatePlot);
    QObject::connect(addNewSparseComponentButton, &QPushButton::clicked,
        this, &AtelierWidget::onAddNewSparseComponent);
    QObject::connect(removeLastSparseComponentButton, &QPushButton::clicked,
        this, &AtelierWidget::onRemoveLastSparseComponentButton);

    processingsGoupboxLayout->addStretch(1);

    drawStandardPCs();
}
