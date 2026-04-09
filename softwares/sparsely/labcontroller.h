#ifndef SPARSEPC_CONTROLLER_HPP
#define SPARSEPC_CONTROLLER_HPP


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

#include "labwidget.h"
#include "preferences.h"

#include "jkqtplotter/jkqtplotter.h"

#include "sparsepc/core.hpp"

namespace sparsely
{
    struct MyClass final
    {
        MyClass()
            :iCandidate{-1}, candidates{}
        {
        }

        MyClass(const MyClass&) = default;
        MyClass& operator=(const MyClass&) = default;

        MyClass(MyClass&&) = default;
        MyClass& operator=(MyClass&&) = default;

        int iCandidate;
        std::unordered_map<sparsepc::Index, sparsepc::Component<double>> candidates;
    };

    class LabController final: public QObject
    {
        Q_OBJECT

    public:
        LabController(LabWidget& labWidget, QObject* parent = nullptr);

        void init(const QString& fileName, bool newProject = true);

        void saveProject(const QString& fileName);

        void loadProject(const QString& fileName);

    public slots:
        void onAddNewSparseComponent();
        void updatePlot(int value);
        void onRemoveLastSparseComponentButton();
        void onSelectionChanged(int index);

    private:
        void drawStandardPCs();
        void computeStandardPCs();
        void drawSparsePCs();
        void connectWidget();

        sparsepc::Matrix<double> m_Sigma;
        sparsepc::Index m_N = 0;

        std::vector<std::reference_wrapper<sparsepc::Component<double>>> m_ValidatedComponents;
        std::vector<MyClass> m_SparsePCs;
        std::vector<MyClass> m_StandardPCs;

        double m_CummulativeVarianceStandardPCs;
        double m_CummulativeVarianceSparsePCs;

        Preferences m_Preferences;

        std::vector<std::unique_ptr<QLibrary>> m_DynamicLibSolverLoaders;
        QVector<QString> m_DynamicLibSolverNames;

        LabWidget& m_LabWidget;
    };
}
#endif //SPARSEPC_CONTROLLER_HPP
