#include "labmainwindow.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QWidget>

#include "labcontroller.h"
#include "labpreferencesdialog.h"
#include "labwidget.h"
#include "version.h"

namespace
{
    void lauchMsgBox(const QString &msg)
    {
        QMessageBox msgBox;
        msgBox.setText(msg);
        msgBox.exec();
    }

    void saveLastFolder(const QString &path)
    {
        QSettings settings(
            QString::fromStdString(std::string(sparsely::metadata::appVendor)),
            QString::fromStdString(std::string(sparsely::metadata::appName)));
        settings.setValue("lastFolder", path);
    }

    QString getLastFolder()
    {
        const QSettings settings(
            QString::fromStdString(std::string(sparsely::metadata::appVendor)),
            QString::fromStdString(std::string(sparsely::metadata::appName)));
        const auto defaultPath =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        return settings.value("lastFolder", defaultPath).toString();
    }
} // namespace

namespace sparsely
{
    LabMainWindow::LabMainWindow(QWidget *parent) : QMainWindow(parent)
    {
        this->setWindowTitle(
            QString::fromStdString(std::string(sparsely::metadata::appTitle)));
        this->setMinimumSize(640, 500);
        auto *newAction = new QAction(
            QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), tr("&New"), this);
        newAction->setShortcuts(QKeySequence::New);
        newAction->setStatusTip(tr("Create a new project"));
        QObject::connect(newAction, &QAction::triggered, this,
                         &LabMainWindow::newFile);
        auto *openAction =
            new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
                        tr("&Open..."), this);
        openAction->setShortcuts(QKeySequence::Open);
        openAction->setStatusTip(tr("Open an existing project"));
        QObject::connect(openAction, &QAction::triggered, this,
                         &LabMainWindow::open);
        auto *saveAction =
            new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave),
                        tr("&Save"), this);
        saveAction->setShortcuts(QKeySequence::Save); // Usually Ctrl+S
        saveAction->setStatusTip(tr("Save project to disk"));
        QObject::connect(saveAction, &QAction::triggered, this,
                         &LabMainWindow::save);
        auto *quitAction = new QAction(tr("&Quit"), this);
        quitAction->setShortcuts(QKeySequence::Quit);
        quitAction->setStatusTip(tr("Quit"));
        QObject::connect(quitAction, &QAction::triggered, this,
                         &LabMainWindow::close);
        auto *fileMenu = menuBar()->addMenu(tr("&File"));
        fileMenu->addAction(newAction);
        fileMenu->addAction(openAction);
        fileMenu->addAction(saveAction);
        // fileMenu->addSeparator();
        fileMenu->addAction(quitAction);
        auto *preferencesAction = new QAction(tr("&Preferences..."), this);
        preferencesAction->setShortcuts(QKeySequence::Preferences);
        preferencesAction->setMenuRole(QAction::PreferencesRole);
        QObject::connect(preferencesAction, &QAction::triggered, this,
                         &LabMainWindow::showPreferences);
        auto *editMenu = menuBar()->addMenu(tr("&Edit"));
        editMenu->addAction(preferencesAction);
        auto *mainWidget = new QWidget(this);
        m_MainWidgetStackedLayout = new QStackedLayout(mainWidget);
        m_MainWidgetStackedLayout->setStackingMode(QStackedLayout::StackOne);
        m_LabWidget = new LabWidget(this);
        m_LabController = new LabController(m_LabModel, *m_LabWidget, this);
        m_MainWidgetStackedLayout->addWidget(m_LabWidget);
        m_LabController->welcome();
        setCentralWidget(mainWidget);
    }

    void LabMainWindow::newFile(bool checked)
    {
        if (!m_LabWidget || !m_LabController)
        { // bad news
            return;
        }

        if (!m_LabController->projectIsEmpty())
        {
            lauchMsgBox("Quit current project first.");
            return;
        }

        const auto lastDir = getLastFolder();
        const auto fileName = QFileDialog::getOpenFileName(
            this, tr("Open File"), lastDir, tr("Text Files (*.csv)"));

        if (!fileName.isEmpty())
        {
            saveLastFolder(QFileInfo(fileName).dir().path());
            if (!m_LabController->init(fileName, true))
            {
                lauchMsgBox("Create new project failed.");
            }
        }
    }

    void LabMainWindow::open(bool checked)
    {
        if (!m_LabWidget || !m_LabController)
        { // bad news
            return;
        }

        if (!m_LabController->projectIsEmpty())
        {
            lauchMsgBox("Quit current project first.");
            return;
        }

        const auto lastDir = getLastFolder();
        const auto fileName = QFileDialog::getOpenFileName(
            this, tr("Open File"), lastDir, tr("Files (*.sparsely)"));

        if (!fileName.isEmpty())
        {
            saveLastFolder(QFileInfo(fileName).dir().path());
            if (!m_LabController->init(fileName, false))
            {
                lauchMsgBox("Load project failed.");
            }
        }
    }

    void LabMainWindow::save(bool checked)
    {
        if (!m_LabWidget || !m_LabController)
        { // bad news
            return;
        }

        if (m_LabController->projectIsEmpty())
        {
            lauchMsgBox("Nothing to save. Empty project.");
            return;
        }
        const auto lastDir = getLastFolder();
        const auto fileName = QFileDialog::getSaveFileName(
            this, tr("Save File"), lastDir, tr("Files (*.sparsely)"));

        if (!fileName.isEmpty())
        {
            saveLastFolder(QFileInfo(fileName).dir().path());
            if (!m_LabController->saveProject(fileName))
            {
                lauchMsgBox("Save project failed.");
            }
        }
    }

    void LabMainWindow::close(bool checked) const { QCoreApplication::quit(); }

    void LabMainWindow::closeEvent(QCloseEvent *event)
    {
        save(true);
        if (event)
        {
            event->accept();
        }
    }

    void LabMainWindow::showPreferences()
    {
        PreferencesDialog dialog(this);
        dialog.exec();
    }
} // namespace sparsely
