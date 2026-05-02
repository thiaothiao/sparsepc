#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QSplashScreen>
#include <QTextStream>
#include <QtLogging>

#include <labmainwindow.h>

#include <thread>

#include <infos.h>

namespace
{
    void messageHandler(QtMsgType type, const QMessageLogContext &context,
                        const QString &message)
    {
        QFile logFile("sparsely.log");
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Append |
                         QIODevice::Text))
        {
            QTextStream out(&logFile);
            out << QDateTime::currentDateTime().toString(
                "yyyy-MM-dd hh:mm:ss ");
            switch (type)
            {
            case QtDebugMsg:
                out << "DEBUG: ";
                break;
            case QtWarningMsg:
                out << "WARNING: ";
                break;
            case QtCriticalMsg:
                out << "CRITICAL: ";
                break;
            case QtFatalMsg:
                out << "FATAL: ";
                break;
            case QtInfoMsg:
                out << "INFO: ";
                break;
            }
            out << message << " (" << context.file << ":" << context.line << ")"
                << Qt::endl;
        }
    }
} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qInstallMessageHandler(messageHandler);
    qInfo() << "Sparsely Start";

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
