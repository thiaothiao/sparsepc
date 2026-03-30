#include "AtelierMainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    AtelierMainWindow w;
    w.show();
    return app.exec();
}
