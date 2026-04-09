#ifndef SPARSEPC_ATELIER_MAINWINDOW_HPP
#define SPARSEPC_ATELIER_MAINWINDOW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QStackedLayout>

#include "labcontroller.h"
#include "labwidget.h"

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
        QWidget* m_WelcomeWidget;
        LabWidget* m_LabWidget;
        LabController* m_LabController;
        QStackedLayout* m_MainWidgetStackedLayout;
    };
}

#endif //SPARSEPC_ATELIER_MAINWINDOW_HPP
