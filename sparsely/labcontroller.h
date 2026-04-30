#pragma once

#include <functional>

#include <QObject>

#include <labpreferences.h>

class QString;
namespace sparsely
{
    class LabModel;
    class LabWidget;

    class LabController final : public QObject
    {
        Q_OBJECT

      public:
        LabController(LabModel &labModel, LabWidget &labWidget,
                      QObject *parent = nullptr);

        void welcome();
        bool init(const QString &fileName, bool newProject = true);
        bool saveProject(const QString &fileName) const;
        bool loadProject(const QString &fileName);
        bool projectIsEmpty() const { return m_N == 0; }

      public slots:
        void onAddNewComponent();
        void onValueChanged(int value);
        void onRemoveLastComponent();
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
