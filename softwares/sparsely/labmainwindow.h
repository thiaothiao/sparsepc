#ifndef SPARSEPC_MAINWINDOW_HPP
#define SPARSEPC_MAINWINDOW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QStackedLayout>

#include "labmodel.h"
#include "labwidget.h"
#include "labcontroller.h"

namespace sparsely
{
    class LabMainWindow : public QMainWindow
    {
        Q_OBJECT

    public slots:
        void newFile(bool checked);
        void open(bool checked);
        void save(bool checked);
        void close(bool checked);
        void showPreferences();

    public:
        LabMainWindow(QWidget *parent = nullptr);

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        QStackedLayout* m_MainWidgetStackedLayout;
        QWidget* m_WelcomeWidget;

        LabModel m_LabModel;
        LabWidget* m_LabWidget;
        LabController* m_LabController;
    };
}

#endif //SPARSEPC_MAINWINDOW_HPP
