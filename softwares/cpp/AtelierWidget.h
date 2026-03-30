#ifndef SPARSEPC_ATELIER_WIDGET_HPP
#define SPARSEPC_ATELIER_WIDGET_HPP

#include <vector>

#include <QWidget>
#include <QTimer>
#include <QColor>
#include <QSlider>

#include "jkqtplotter/jkqtplotter.h"
#include "jkqtplotter/graphs/jkqtpfilledcurve.h"
#include "jkqtplotter/graphs/jkqtpscatter.h"

#include "sparsepc/core.hpp"

class AtelierWidget : public QWidget
{
    Q_OBJECT

public:
    AtelierWidget(QWidget* parent = nullptr);

void drawPCs();

public slots:
    void onAddNewSparseComponent();
    void updatePlot(int iCandidate);
    void onClearSparseCandidates();

private:
    sparsepc::Matrix<double> m_Sigma;
    size_t m_ColumnX = 0;
    int m_ICandidate = -1;
    std::vector<size_t> m_SPCColumns;
    std::vector<sparsepc::Component<double>> m_Candidates;
    std::vector<sparsepc::Component<double>> m_ValidatedComponents;
    JKQTPlotter* m_Plotter=nullptr;
    QSlider* m_SparsityLevelSlider=nullptr;
    QVector<JKQTPXYLineGraph*> m_PCGraphs;
    QVector<JKQTPXYLineGraph*> m_SPCGraphs;
    QVector<QColor> m_Colors;
    QTimer m_DataTimer;
};

#endif //SPARSEPC_ATELIER_WIDGET_HPP
