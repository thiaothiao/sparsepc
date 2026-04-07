#include "AtelierMainWindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QDir>
#include <QSplashScreen>

#include <thread>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QDir(QDir::currentPath() + "/images").absoluteFilePath("logo_transparent.png")));

    QPixmap pixmap(QDir(QDir::currentPath() + "/images").absoluteFilePath("welcome_transparent.png"));
    QSplashScreen splash(pixmap);
    splash.setMask(pixmap.mask());

    splash.show();

    {
        QPixmap pixmp(QDir(QDir::currentPath() + "/images").absoluteFilePath("logo.png"));
        pixmp.setMask(pixmp.createMaskFromColor(Qt::white));
        pixmp.save(QDir(QDir::currentPath() + "/images").absoluteFilePath("logo_transparent.png"));
    }

    {
        QPixmap pixmp(QDir(QDir::currentPath() + "/images").absoluteFilePath("welcome.png"));
        pixmp.setMask(pixmp.createMaskFromColor(Qt::white));
        pixmp.save(QDir(QDir::currentPath() + "/images").absoluteFilePath("welcome_transparent.png"));
    }

    {
        QPixmap pixmp(QDir(QDir::currentPath() + "/images").absoluteFilePath("text.png"));
        pixmp.setMask(pixmp.createMaskFromColor(Qt::white));
        pixmp.save(QDir(QDir::currentPath() + "/images").absoluteFilePath("text_transparent.png"));
    }

    app.processEvents();
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
    }

    AtelierMainWindow w;
    w.show();

    splash.finish(&w);
    return app.exec();
}
