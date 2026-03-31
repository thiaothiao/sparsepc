#include "sparsepc/version.hpp"

#include "AtelierMainWindow.h"
#include "AtelierWidget.h"

AtelierMainWindow::AtelierMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    constexpr std::string_view version = SPARSEPC_MACRO_STRINGIFY(SPARSEPC_VERSION);
    this->setWindowTitle("SPARSELY Software - SPARSE principal component LaboratorY");
    this->setMinimumSize(640, 500);
    auto* main_widget = new AtelierWidget(this);
    this->setCentralWidget(main_widget);
}
