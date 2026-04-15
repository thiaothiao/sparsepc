#ifndef SPARSEPC_CONTROLLER_HPP
#define SPARSEPC_CONTROLLER_HPP

#include <functional>

#include <QObject>
#include <QString>

#include "labmodel.h"
#include "labpreferences.h"
#include "labwidget.h"

namespace sparsely
{
    class LabController final : public QObject
    {
        Q_OBJECT

      public:
        LabController(LabModel &labModel, LabWidget &labWidget,
                      QObject *parent = nullptr);

        void welcome();
        void init(const QString &fileName, bool newProject = true);
        void saveProject(const QString &fileName) const;
        void loadProject(const QString &fileName);
        bool projectIsEmpty() const { return m_N == 0; }

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
        std::reference_wrapper<LabWidget> m_LabWidget;
        std::reference_wrapper<LabModel> m_LabModel;
    };
} // namespace sparsely
#endif // SPARSEPC_CONTROLLER_HPP
