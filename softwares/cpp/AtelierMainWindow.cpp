#include "AtelierMainWindow.h"

#include "AtelierWidget.h"

AtelierMainWindow::AtelierMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("SPARSEli- SPARse principal component SELector LIghtweight software");
    this->setMinimumSize(640, 500);
    auto* main_widget = new AtelierWidget(this);
    this->setCentralWidget(main_widget);

}
