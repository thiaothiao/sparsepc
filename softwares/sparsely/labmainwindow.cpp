#include "labmainwindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QDebug>

#include "sparsepc/version.hpp"

#include "labpreferencesdialog.h"

namespace sparsely
{
    LabMainWindow::LabMainWindow(QWidget* parent)
        : QMainWindow(parent)
    {
        constexpr std::string_view version = SPARSEPC_MACRO_STRINGIFY(SPARSEPC_VERSION);
        this->setWindowTitle("SPARSELY Software - SPARSE principal component LaboratorY");
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

        // Connect to a slot that opens your settings dialog
        QObject::connect(preferencesAction, &QAction::triggered, this,
                         &LabMainWindow::showPreferences);

        // Add it to a menu (e.g., the Edit menu)
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
        const QString fileName = QFileDialog::getOpenFileName(
            this,                        // Parent widget
            "Open File",                 // Dialog title
            "/home",                     // Starting directory
            "Text Files (*.csv)" // File filters
            );

        if (!fileName.isEmpty())
        {
            if(m_LabWidget)
            {
                m_LabController->init(fileName, true);
                m_MainWidgetStackedLayout->setCurrentWidget(m_LabWidget);
            }
        }
    }

    void LabMainWindow::open(bool checked)
    {
        const QString fileName = QFileDialog::getOpenFileName(
            this,                        // Parent widget
            "Open File",                 // Dialog title
            "/home",                     // Starting directory
            "Files (*.sparsely)" // File filters
            );

        if (!fileName.isEmpty())
        {
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
            const auto fileName = QFileDialog::getSaveFileName(this,
                tr("Save File"), "/home/user/data.sparsely", tr("Files (*.sparsely)"));

            if (!fileName.isEmpty())
            {
                m_LabController->saveProject(fileName);
            }
        }
    }

    void LabMainWindow::close(bool checked)
    {
        save(true);
        QCoreApplication::quit();
    }

    void LabMainWindow::closeEvent(QCloseEvent *event)
    {// closing via X button
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
            // Appliquer les nouveaux réglages ici
            qDebug() << "Accepted";
        }
    }
}
