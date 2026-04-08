#include "labmainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QDir>
#include <QSplashScreen>

#include <thread>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/logo_transparent.png"));
    QPixmap pixmap(":/images/welcome_transparent.png");
    QSplashScreen splash(pixmap);
    splash.setMask(pixmap.mask());

    splash.show();

    app.processEvents();
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
    }

    sparsely::LabMainWindow w;
    w.show();

    splash.finish(&w);
    return app.exec();
}
