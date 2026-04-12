#include "labmainwindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

#include "version.h"
#include "labpreferencesdialog.h"

namespace
{
    void saveLastFolder(const QString& path)
    {
        QSettings settings(QString::fromStdString(std::string(sparsely::meta::VENDOR)), "Sparsely");
        settings.setValue("lastFolder", path);
    }

    QString getLastFolder()
    {
        const QSettings settings(QString::fromStdString(std::string(sparsely::meta::VENDOR)), "Sparsely");

        const auto defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

        return settings.value("lastFolder", defaultPath).toString();
    }

}

namespace sparsely
{
    LabMainWindow::LabMainWindow(QWidget* parent)
        : QMainWindow(parent)
    {
        //this->setWindowTitle(tr("SPARSELY Software - ?????"));
        this->setWindowTitle(QString::fromStdString(std::string(sparsely::meta::PROJECT_DESCRIPTION)));
        this->setMinimumSize(640, 500);

         auto* newAction = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew),
            tr("&New"), this);
        newAction->setShortcuts(QKeySequence::New);
        newAction->setStatusTip(tr("Create a new project"));
        QObject::connect(newAction, &QAction::triggered, this, &LabMainWindow::newFile);

        auto* openAction = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
            tr("&Open..."), this);
        openAction->setShortcuts(QKeySequence::Open);
        openAction->setStatusTip(tr("Open an existing project"));
        QObject::connect(openAction, &QAction::triggered, this, &LabMainWindow::open);

        auto* saveAction = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave),
            tr("&Save"), this);
        saveAction->setShortcuts(QKeySequence::Save); // Usually Ctrl+S
        saveAction->setStatusTip(tr("Save project to disk"));
        QObject::connect(saveAction, &QAction::triggered, this, &LabMainWindow::save);

        auto* quitAction = new QAction(tr("&Quit"), this);
        quitAction->setShortcuts(QKeySequence::Quit);
        quitAction->setStatusTip(tr("Quit"));
        QObject::connect(quitAction, &QAction::triggered, this, &LabMainWindow::close);

        auto* fileMenu = menuBar()->addMenu(tr("&File"));
        fileMenu->addAction(newAction);
        fileMenu->addAction(openAction);
        fileMenu->addAction(saveAction);
        //fileMenu->addSeparator();
        fileMenu->addAction(quitAction);

        auto *preferencesAction = new QAction(tr("&Preferences..."), this);
        preferencesAction->setShortcuts(QKeySequence::Preferences);
        preferencesAction->setMenuRole(QAction::PreferencesRole);

        QObject::connect(preferencesAction, &QAction::triggered, this,
                         &LabMainWindow::showPreferences);

        auto* editMenu = menuBar()->addMenu(tr("&Edit"));
        editMenu->addAction(preferencesAction);

        auto* mainWidget = new QWidget(this);

        m_MainWidgetStackedLayout = new QStackedLayout(mainWidget);
        m_MainWidgetStackedLayout->setStackingMode(QStackedLayout::StackOne);

        m_WelcomeWidget = new QWidget(this);
        auto* welcomeWidgeLayout = new QHBoxLayout(m_WelcomeWidget);

        auto* topLevelLabel = new QLabel(this);
        QPixmap pixmap(":/images/welcome.png");
        topLevelLabel->setPixmap(pixmap);
        topLevelLabel->setMask(pixmap.mask());
        welcomeWidgeLayout->addWidget(topLevelLabel, 0, Qt::AlignCenter);

        m_WelcomeWidget->setStyleSheet("background-color: white;");

        m_LabWidget = new LabWidget(this);

        m_LabController = new LabController(m_LabModel, *m_LabWidget, this);

        m_MainWidgetStackedLayout->addWidget(m_WelcomeWidget);
        m_MainWidgetStackedLayout->addWidget(m_LabWidget);

        m_MainWidgetStackedLayout->setCurrentWidget(m_WelcomeWidget);

        setCentralWidget(mainWidget);
    }

    void LabMainWindow::newFile(bool checked)
    {
        const auto lastDir = getLastFolder();

        const auto fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open File"),
            lastDir,
            tr("Text Files (*.csv)")
            );

        if (!fileName.isEmpty())
        {
            saveLastFolder(QFileInfo(fileName).dir().path());
            if(m_LabWidget)
            {
                m_LabController->init(fileName, true);
                m_MainWidgetStackedLayout->setCurrentWidget(m_LabWidget);
            }
        }
    }

    void LabMainWindow::open(bool checked)
    {
        const auto lastDir = getLastFolder();

        const auto fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open File"),
            lastDir,
            tr("Files (*.sparsely)")
            );

        if (!fileName.isEmpty())
        {
            saveLastFolder(QFileInfo(fileName).dir().path());
            if(m_LabWidget)
            {
                m_LabController->init(fileName, false);
                m_MainWidgetStackedLayout->setCurrentWidget(m_LabWidget);
            }
        }
    }

    void LabMainWindow::save(bool checked)
    {
        if(m_LabWidget)
        {
            if(m_LabController->projectIsEmpty())
            {
                return;
            }

            const auto lastDir = getLastFolder();

            const auto fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save File"),
                lastDir,
                tr("Files (*.sparsely)"));

            if (!fileName.isEmpty())
            {
                saveLastFolder(QFileInfo(fileName).dir().path());
                m_LabController->saveProject(fileName);
            }
        }
    }

    void LabMainWindow::close(bool checked) const
    {
        QCoreApplication::quit();
    }

    void LabMainWindow::closeEvent(QCloseEvent *event)
    {
        save(true);
        if(event)
        {
            event->accept();
        }
    }

    void LabMainWindow::showPreferences()
    {
        PreferencesDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted)
        {
            qDebug() << "Accepted";
        }
    }
}
