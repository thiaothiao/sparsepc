#ifndef SPARSEPC_ATELIER_MAINWINDOW_HPP
#define SPARSEPC_ATELIER_MAINWINDOW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QStackedLayout>

#include "AtelierWidget.h"

class AtelierMainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void newFile(bool checked);
    void open(bool checked);
    void save(bool checked);
    void close(bool checked);

public:
    AtelierMainWindow(QWidget *parent = nullptr);

private:
    QWidget* m_WelcomeWidget;
    AtelierWidget* m_AtelierWidget;
    QStackedLayout* m_MainWidgetStackedLayout;
};

#endif //SPARSEPC_ATELIER_MAINWINDOW_HPP
