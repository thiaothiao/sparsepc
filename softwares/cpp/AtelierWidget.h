#ifndef SPARSEPC_ATELIER_WIDGET_HPP
#define SPARSEPC_ATELIER_WIDGET_HPP

#include <vector>

#include <QWidget>
#include <QTimer>
#include <QColor>

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
    void onComputeSparseCandidates();
    void updatePlot(int iCandidate);

private:
    sparsepc::Matrix<double> m_Sigma;
    size_t m_ColumnX = 0;
    std::vector<size_t> m_SPCColumns;
    std::vector<sparsepc::Component<double>> m_Candidates;
    JKQTPlotter* m_Plotter=nullptr;
    QVector<JKQTPXYLineGraph*> m_PCGraphs;
    QVector<JKQTPXYLineGraph*> m_SPCGraphs;
    QVector<QColor> m_Colors;
    QTimer m_DataTimer;
};

#endif //SPARSEPC_ATELIER_WIDGET_HPP
