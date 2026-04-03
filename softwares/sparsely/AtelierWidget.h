#ifndef SPARSEPC_ATELIER_WIDGET_HPP
#define SPARSEPC_ATELIER_WIDGET_HPP

#include <vector>
#include <functional>
#include <unordered_map>

#include <QWidget>
#include <QSlider>
#include <QVector>
#include <QProgressBar>
#include <QStackedLayout>
#include <QCombobox>
#include <QGroupBox>

#include "jkqtplotter/jkqtplotter.h"
#include "jkqtplotter/graphs/jkqtpfilledcurve.h"
#include "jkqtplotter/graphs/jkqtpscatter.h"
#include "jkqtplotter/graphs/jkqtpbarchart.h"

#include "sparsepc/core.hpp"

struct MyClass final
{
    MyClass()
        :iCandidate{-1}, pcColumn{0}, candidates{},
        pcGraph{nullptr}, color{}
    {
    }

    MyClass(const MyClass&) = default;
    MyClass& operator=(const MyClass&) = default;

    MyClass(MyClass&&) = default;
    MyClass& operator=(MyClass&&) = default;

    int iCandidate;
    std::size_t pcColumn;
    std::unordered_map<sparsepc::Index, sparsepc::Component<double>> candidates;
    JKQTPFilledCurveXGraph* pcGraph;
    QString color;
};

class AtelierWidget : public QWidget
{
    Q_OBJECT

public:
    AtelierWidget(QWidget* parent = nullptr);

void drawStandardPCs();

void init(sparsepc::Matrix<double>&& sigma);

public slots:
    void onAddNewSparseComponent();
    void updatePlot(int value);
    void onRemoveLastSparseComponentButton();
    void updateSliderTitle(int value);
    void updateProgressBarTitle(int value);
    void save(bool checked);

private:
    sparsepc::Matrix<double> m_Sigma;

    std::size_t m_ColumnX;

    std::vector<std::reference_wrapper<sparsepc::Component<double>>> m_ValidatedComponents;

    std::vector<MyClass> m_SparsePCs;

    std::vector<MyClass> m_StandardPCs;

    double m_CummulativeVarianceStandardPCs;
    double m_CummulativeVarianceSparsePCs;

    JKQTPlotter* m_Plotter;
    QGroupBox* m_SliderGroupBox;
    QGroupBox* m_ProgressBarGroupBox;
    QSlider* m_Slider;
    QProgressBar* m_ProgressBar;
    QStackedLayout* m_SliderOrProgressBarWidgetStackedLayout;
    QComboBox* m_MethodComboBox;

    QVector<QString> m_Colors;
};

#endif //SPARSEPC_ATELIER_WIDGET_HPP
