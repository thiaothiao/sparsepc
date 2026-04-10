#ifndef SPARSEPC_CONTROLLER_HPP
#define SPARSEPC_CONTROLLER_HPP

#include <QString>
#include <QObject>

#include "labwidget.h"
#include "labmodel.h"
#include "labpreferences.h"

namespace sparsely
{
    class LabController final: public QObject
    {
        Q_OBJECT

    public:
        LabController(LabModel& labModel, LabWidget& labWidget, QObject* parent = nullptr);

        void init(const QString& fileName, bool newProject = true);

        void saveProject(const QString& fileName) const;
        void loadProject(const QString& fileName);
        bool projectIsEmpty() const;

    public slots:
        void onAddNewSparseComponent();
        void updatePlot(int value);
        void onRemoveLastSparseComponentButton();
        void onSelectionChanged(int index);

    private:
        void addStandardPCGraphs();
        void addSparsePCGraphs();
        void connectWidget();

        int m_N;
        Preferences m_Preferences;
        LabWidget& m_LabWidget;
        LabModel& m_LabModel;
    };
}
#endif //SPARSEPC_CONTROLLER_HPP
