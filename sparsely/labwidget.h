#pragma once

#include <cstddef>
#include <vector>

#include <QPen>
#include <QString>
#include <QVector>
#include <QWidget>

#include <labenums.h>

class QGroupBox;
class QSlider;
class QStackedLayout;
class QComboBox;
class QPushButton;
class QRadioButton;

class JKQTPPlotElement;
class JKQTPlotter;

namespace Sparsepc
{
    template <std::floating_point ScalarType> struct Component;
}

namespace Sparsely
{
    struct Preferences;

    struct GraphInfo final
    {
        GraphInfo() : xColumn{0}, pcColumn{0}, color{}, graph{nullptr} {}

        GraphInfo(const GraphInfo &) = default;
        GraphInfo &operator=(const GraphInfo &) = default;
        GraphInfo(GraphInfo &&) = default;
        GraphInfo &operator=(GraphInfo &&) = default;

        std::size_t xColumn;
        std::size_t pcColumn;
        QString color;
        JKQTPPlotElement *graph;
    };

    // The view part of the MVC.
    class LabWidget : public QWidget
    {
        Q_OBJECT

      public:
        LabWidget(Enums::ScaleType scale, QWidget *parent = nullptr);

      public slots:
        void updateSliderTitle(int value);

      public:
        void welcome(const QString &addonName);
        void updateWidget(const Preferences &preferences,
                          const QString &addonName,
                          const QVector<QString> &header);
        void lauchMessageBox(const QString &text);
        void setPlotUpdateEnabled(bool enable);
        void redrawPlot();
        void zoomToFit();
        void addStandardPCGraph(const Preferences &preferences,
                                const Sparsepc::Component<double> &component,
                                const QString &legend);
        void addSparsePCGraph(const Preferences &preferences,
                              const Sparsepc::Component<double> &component,
                              const QString &legend);
        void
        updateLastSparsePCGraph(const Sparsepc::Component<double> &component,
                                const QString &legend);
        void removeLastSparsePCGraph(int sliderValue);
        void removeLastStandardPCGraph();
        void clearAlls();
        void reInitSlider(int value);
        void setSliderColor(const QString &colorString);
        void setN(int n);

        std::vector<GraphInfo> m_SparsePCGraphs;
        std::vector<GraphInfo> m_StandardPCGraphs;
        QGroupBox *m_SliderGroupBox;
        QSlider *m_Slider;
        QStackedLayout *m_SliderOrProgressBarWidgetStackedLayout;
        QStackedLayout *m_ProcessingsGoupboxStackedLayout;
        QComboBox *m_MethodComboBox;
        QComboBox *m_PlotTypeComboBox;
        QPushButton *m_AddNewComponentButton;
        QPushButton *m_RemoveLastComponentButton;
        QRadioButton *m_SparsePCRadioButton;
        QRadioButton *m_StandardPCRadioButton;
        QWidget *m_ButtonsWidget;
        QVector<QString> m_Colors;

      private:
        void addPCGraph(std::vector<GraphInfo> &pcs,
                        const Sparsepc::Component<double> &component,
                        const QString &legend, const Qt::PenStyle &lineStyle,
                        double lineWidthFilledPlot,
                        double lineWidthImpulsesPlot,
                        double fillingColorsAlpha);

        int m_N;
        std::size_t m_ColumnX;
        double m_Xmin;
        double m_Xmax;
        const double m_Ymin;
        const double m_Ymax;
        JKQTPlotter *m_Plotter;
        const Enums::ScaleType m_Scale;
    };
} // namespace Sparsely
