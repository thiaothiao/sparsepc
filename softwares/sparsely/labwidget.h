#ifndef SPARSEPC_WIDGET_HPP
#define SPARSEPC_WIDGET_HPP

#include <cstddef>
#include <vector>
#include <memory>

#include <QString>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QVector>
#include <QProgressBar>
#include <QStackedLayout>
#include <QCombobox>
#include <QLibrary>

#include "labpreferences.h"

#include "jkqtplotter/jkqtplotter.h"

#include "sparsepc/core.hpp"

namespace sparsely
{
    struct MyClass2 final
    {
        MyClass2()
            :xColumn{0}, pcColumn{0}, color{}, graph{nullptr}
        {
        }

        MyClass2(const MyClass2&) = default;
        MyClass2& operator=(const MyClass2&) = default;
        MyClass2(MyClass2&&) = default;
        MyClass2& operator=(MyClass2&&) = default;

        std::size_t xColumn;
        std::size_t pcColumn;
        QString color;
        JKQTPPlotElement* graph;
    };

    class LabWidget : public QWidget
    {
        Q_OBJECT

    public:
        LabWidget(QWidget* parent = nullptr);

    public slots:
        void updateSliderTitle(int value);
        void updateProgressBarTitle(int value);

    public:
        void setPreferences(Preferences* preferences);
        void createWidget(const QString& addonName);
        void sePlotUpdateEnabled(bool enable);
        void redrawPlot();
        void zoomToFit();
        void addStandardPCGraph(const sparsepc::Component<double>& component,
            const QString& cumulativeVarianceString, const QString& curveName);
        void addSparsePCGraph(const sparsepc::Component<double>& component,
            const QString& cumulativeVarianceString, const QString& curveName);
        void updateLastSparsePCGraph(const sparsepc::Component<double>& component,
            const QString& cumulativeVarianceString, const QString& curveName);
        void removeLastSparsePCGraph(int sliderValue);
        void clearAlls();
        void reInitSlider(int value);
        void setSliderColor(const QString& colorString);
        void setProgressBarColor(const QString& colorString);
        void setN(int n);

        std::vector<MyClass2> m_SparsePCGraphs;
        std::vector<MyClass2> m_StandardPCGraphs;
        QGroupBox* m_SliderGroupBox;
        QGroupBox* m_ProgressBarGroupBox;
        QSlider* m_Slider;
        QProgressBar* m_ProgressBar;
        QStackedLayout* m_SliderOrProgressBarWidgetStackedLayout;
        QComboBox* m_MethodComboBox;
        QComboBox* m_PlotTypeComboBox;
        QPushButton* m_AddNewSparseComponentButton;
        QPushButton* m_RemoveLastSparseComponentButton;
        QVector<QString> m_Colors;
        std::vector<std::unique_ptr<QLibrary>> m_DynamicLibSolverLoaders;
        QVector<QString> m_DynamicLibSolverNames;

    private:
        void addPCGraph(std::vector<MyClass2>& pcs, const sparsepc::Component<double>& component,
            const QString& cumulativeVarianceString, const QString& curveName,
            const Qt::PenStyle& lineStyle, double lineWidthFilledPlot,
            double lineWidthImpulsesPlot, double fillingColorsAlpha);

        int m_N;
        std::size_t m_ColumnX;
        Preferences* m_Preferences;
        JKQTPlotter* m_Plotter;
    };
}
#endif //SPARSEPC_WIDGET_HPP
