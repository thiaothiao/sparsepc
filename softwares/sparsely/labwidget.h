#ifndef SPARSEPC_ATELIER_WIDGET_HPP
#define SPARSEPC_ATELIER_WIDGET_HPP

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#include <QString>
#include <QWidget>
#include <QGroupBox>
#include <QSlider>
#include <QVector>
#include <QProgressBar>
#include <QStackedLayout>
#include <QCombobox>
#include <QLibrary>

#include "preferences.h"

#include "jkqtplotter/jkqtplotter.h"

#include "sparsepc/core.hpp"

namespace sparsely
{
    struct MyClass final
    {
        MyClass()
            :iCandidate{-1}, pcColumn{0},
            color{}, candidates{}, pcGraph{nullptr}
        {
        }

        MyClass(const MyClass&) = default;
        MyClass& operator=(const MyClass&) = default;

        MyClass(MyClass&&) = default;
        MyClass& operator=(MyClass&&) = default;

        int iCandidate;
        std::size_t pcColumn;
        QString color;
        std::unordered_map<sparsepc::Index, sparsepc::Component<double>> candidates;
        JKQTPPlotElement* pcGraph;
    };

    class LabWidget : public QWidget
    {
        Q_OBJECT

    public:
        LabWidget(QWidget* parent = nullptr);

        void init(const QString& fileName, bool newProject = true);

        void saveProject(const QString& fileName);

        void loadProject(const QString& fileName);

    public slots:
        void onAddNewSparseComponent();
        void updatePlot(int value);
        void onRemoveLastSparseComponentButton();
        void updateSliderTitle(int value);
        void updateProgressBarTitle(int value);
        void onSelectionChanged(int index);

    private:
        void drawStandardPCs();
        void computeStandardPCs();
        void drawSparsePCs();
        void clearAllPlots();
        void createWidget();
        void zoomToFit();

        sparsepc::Matrix<double> m_Sigma;
        sparsepc::Index m_N = 0;
        std::size_t m_ColumnX;
        std::vector<std::reference_wrapper<sparsepc::Component<double>>> m_ValidatedComponents;
        std::vector<MyClass> m_SparsePCs;
        std::vector<MyClass> m_StandardPCs;
        double m_CummulativeVarianceStandardPCs;
        double m_CummulativeVarianceSparsePCs;

        Preferences m_Preferences;

        JKQTPlotter* m_Plotter;
        QGroupBox* m_SliderGroupBox;
        QGroupBox* m_ProgressBarGroupBox;
        QSlider* m_Slider;
        QProgressBar* m_ProgressBar;
        QStackedLayout* m_SliderOrProgressBarWidgetStackedLayout;
        QComboBox* m_MethodComboBox;
        QComboBox* m_PlotTypeComboBox;

        QVector<QString> m_Colors;

        std::vector<std::unique_ptr<QLibrary>> m_DynamicLibSolverLoaders;
        QVector<QString> m_DynamicLibSolverNames;
    };
}
#endif //SPARSEPC_ATELIER_WIDGET_HPP
