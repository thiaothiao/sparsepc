#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMutex>
#include <QPixmap>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QTextStream>
#include <QtLogging>

#include <labmainwindow.h>

#include <thread>

#include <infos.h>

namespace
{
    QString retrieveLogFilePath()
    {
        const QString baseName("sparsely.log");
        const auto dataPath = QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation);
        if (!QDir().mkpath(dataPath))
        { // critical
            return baseName;
        }
        QDir dir(dataPath);
        if (!dir.exists("logs"))
        {
            if (!dir.mkdir("logs"))
            { // critical
                return baseName;
            }
        }
        if (!dir.cd("logs"))
        { // critical
            return baseName;
        }
        return dir.absoluteFilePath(baseName);
    }

    void messageHandler(QtMsgType type, const QMessageLogContext &context,
                        const QString &message)
    {
        static QMutex mutex;
        QMutexLocker locker(&mutex);

        const auto logFilePath = retrieveLogFilePath();
        QFile logFile(logFilePath);
        const qint64 maximumSize = 1024 * 1024; // 1 MB limit
        const int maximumNumberOfFiles = 3;

        // rotation logic
        if (logFile.exists() && logFile.size() > maximumSize)
        {
            logFile.close();
            // shift old logs
            // log.2 -> log.3, log.1 -> log.2, log -> log.1, etc.
            for (int i = maximumNumberOfFiles - 1; i >= 1; --i)
            {
                [[maybe_unused]]
                const auto bRemove =
                    QFile::remove(logFilePath + "." + QString::number(i + 1));
                [[maybe_unused]]
                const auto bRename =
                    QFile::rename(logFilePath + "." + QString::number(i),
                                  logFilePath + "." + QString::number(i + 1));
            }
            [[maybe_unused]] const auto bRename =
                QFile::rename(logFilePath, logFilePath + ".1");
        }

        if (logFile.open(QIODevice::WriteOnly | QIODevice::Append |
                         QIODevice::Text))
        {
            const QString timestamp =
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

            QString typeAsString("UNKNOWN: ");
            switch (type)
            {
            case QtDebugMsg:
                typeAsString = "DEBUG: ";
                break;
            case QtWarningMsg:
                typeAsString = "WARNING: ";
                break;
            case QtCriticalMsg:
                typeAsString = "CRITICAL: ";
                break;
            case QtFatalMsg:
                typeAsString = "FATAL: ";
                break;
            case QtInfoMsg:
                typeAsString = "INFO: ";
                break;
            }
            // Note: context.file & context.line may be null/0 in Release builds
            const QString fileName =
                context.file ? QString(context.file) : "unknown";
            const auto lineNumber = context.line;

            QTextStream out(&logFile);
            out << "[" << timestamp << "] " << typeAsString << " (" << fileName
                << ":" << lineNumber << "): " << message << "\n";
        }
    }
} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qInstallMessageHandler(messageHandler);
    qInfo() << "Sparsely start";

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
