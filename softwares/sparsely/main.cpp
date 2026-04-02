#include "AtelierMainWindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QSplashScreen>

#include <thread>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(R"(C:\Users\thiao\Pictures\sparsely\logo1.png)"));

    QPixmap pixmap(R"(C:\Users\thiao\Pictures\sparsely\logo1_text.png)");
    QSplashScreen splash(pixmap);
    splash.show();

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
