#ifndef SPARSEPC_ATELIER_WIDGET_HPP
#define SPARSEPC_ATELIER_WIDGET_HPP

#include <vector>
#include <functional>

#include <QWidget>
#include <QSlider>
#include <QTimer>
#include <QColor>
#include <QVector>
#include <QProgressBar>

#include "jkqtplotter/jkqtplotter.h"
#include "jkqtplotter/graphs/jkqtpfilledcurve.h"
#include "jkqtplotter/graphs/jkqtpscatter.h"

#include "sparsepc/core.hpp"

struct MyClass final
{
    MyClass()
        :iCandidate{-1}, sPCColumn{0}, candidates{},
        sPCGraph{nullptr}, color{}
    {
    }

    MyClass(const MyClass&) = default;
    MyClass& operator=(const MyClass&) = default;

    MyClass(MyClass&&) = default;
    MyClass& operator=(MyClass&&) = default;

    int iCandidate;
    std::size_t sPCColumn;
    std::vector<sparsepc::Component<double>> candidates;
    JKQTPXYLineGraph* sPCGraph;
    QColor color;
};

class AtelierWidget : public QWidget
{
    Q_OBJECT

public:
    AtelierWidget(QWidget* parent = nullptr);

void drawStandardPCs();

public slots:
    void onAddNewSparseComponent();
    void updatePlot(int value);
    void onRemoveLastSparseComponentButton();

private:
    sparsepc::Matrix<double> m_Sigma;
    std::size_t m_ColumnX;
    std::vector<std::reference_wrapper<sparsepc::Component<double>>> m_ValidatedComponents;
    std::vector<MyClass> m_MyClasses;
    std::vector<JKQTPXYLineGraph*> m_PCGraphs;

    JKQTPlotter* m_Plotter;
    QSlider* m_SparsityLevelSlider;
    QProgressBar* m_ProgressBar;

    QVector<QColor> m_Colors;
};

#endif //SPARSEPC_ATELIER_WIDGET_HPP
