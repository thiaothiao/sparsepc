#pragma once

#include <QMainWindow>

#include <labenums.h>
#include <labmodel.h>

class QWidget;
class QCloseEvent;

namespace sparsely
{
    class LabWidget;
    class LabController;

    class LabMainWindow final : public QMainWindow
    {
        Q_OBJECT

      public slots:
        bool newFile(bool checked);
        bool open(bool checked);
        Enums::SaveStatus save(bool checked);
        void close(bool checked) const;
        void showPreferences();
        void about();
        void aboutJKQTPlotter();
        void aboutEigen();

      public:
        LabMainWindow(QWidget *parent = nullptr);

      private:
        void closeEvent(QCloseEvent *event) override;
        LabModel m_LabModel;
        LabWidget *m_LabWidget;
        LabController *m_LabController;
    };
} // namespace sparsely
