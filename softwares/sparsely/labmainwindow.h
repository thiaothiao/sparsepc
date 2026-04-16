#ifndef SPARSEPC_MAINWINDOW_HPP
#define SPARSEPC_MAINWINDOW_HPP

#include <QMainWindow>

#include "labmodel.h"

class QWidget;
class QCloseEvent;
class QStackedLayout;

namespace sparsely
{
    class LabWidget;
    class LabController;

    class LabMainWindow final : public QMainWindow
    {
        Q_OBJECT

      public slots:
        void newFile(bool checked);
        void open(bool checked);
        void save(bool checked);
        void close(bool checked) const;
        void showPreferences();

      public:
        LabMainWindow(QWidget *parent = nullptr);

      private:
        void closeEvent(QCloseEvent *event) override;
        QStackedLayout *m_MainWidgetStackedLayout;
        LabModel m_LabModel;
        LabWidget *m_LabWidget;
        LabController *m_LabController;
    };
} // namespace sparsely

#endif // SPARSEPC_MAINWINDOW_HPP
