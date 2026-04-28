#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QPixmap>
#include <QSplashScreen>
#include <labmainwindow.h>

#include <thread>

#include <infos.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle("fusion"); // force fusion style
    app.setApplicationName(
        QString::fromStdString(std::string(sparsely::metadata::appName)));
    app.setOrganizationName(
        QString::fromStdString(std::string(sparsely::metadata::appVendor)));
    app.setOrganizationDomain(
        QString::fromStdString(std::string(sparsely::metadata::appDomain)));
    app.setApplicationVersion(
        QString::fromStdString(std::string(sparsely::metadata::appVersion)));

    app.setWindowIcon(QIcon(":/icons/window.png"));
    QPixmap pixmap(":/images/welcome.png");
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
