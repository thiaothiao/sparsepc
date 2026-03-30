#include "AtelierMainWindow.h"

#include "AtelierWidget.h"

AtelierMainWindow::AtelierMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("SPARSELY- SPARse principal component SElector LIghtweight software");
    this->setMinimumSize(640, 500);
    auto* main_widget = new AtelierWidget(this);
    this->setCentralWidget(main_widget);

}
