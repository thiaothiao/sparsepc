#include "sparsepc/version.hpp"

#include "AtelierMainWindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>

AtelierMainWindow::AtelierMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    constexpr std::string_view version = SPARSEPC_MACRO_STRINGIFY(SPARSEPC_VERSION);
    this->setWindowTitle("SPARSELY Software - SPARSE principal component LaboratorY");
    this->setMinimumSize(640, 500);

     auto* newAction = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew),
        tr("&New"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr("Create a new file"));
    QObject::connect(newAction, &QAction::triggered, this, &AtelierMainWindow::newFile);

    auto* openAction = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
        tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    QObject::connect(openAction, &QAction::triggered, this, &AtelierMainWindow::open);

    auto* saveAction = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave),
        tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save); // Usually Ctrl+S
    saveAction->setStatusTip(tr("Save the document to disk"));
    QObject::connect(saveAction, &QAction::triggered, this, &AtelierMainWindow::save);

    auto* quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcuts(QKeySequence::Quit);
    quitAction->setStatusTip(tr("Quit"));
    QObject::connect(quitAction, &QAction::triggered, this, &AtelierMainWindow::close);

    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    //fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    auto* mainWidget = new QWidget(this);

    m_MainWidgetStackedLayout = new QStackedLayout(mainWidget);
    m_MainWidgetStackedLayout->setStackingMode(QStackedLayout::StackOne);

    m_WelcomeWidget = new QWidget(this);
    auto* welcomeWidgeLayout = new QHBoxLayout(m_WelcomeWidget);

    auto* topLevelLabel = new QLabel(this);
    QPixmap pixmap(R"(C:\Users\thiao\Pictures\sparsely\welcome.png)");
    topLevelLabel->setPixmap(pixmap);
    topLevelLabel->setMask(pixmap.mask());
    welcomeWidgeLayout->addWidget(topLevelLabel);

    m_MainWidgetStackedLayout->addWidget(m_WelcomeWidget);

    m_AtelierWidget = new AtelierWidget(this);
    m_MainWidgetStackedLayout->addWidget(m_AtelierWidget);

    m_MainWidgetStackedLayout->setCurrentWidget(m_WelcomeWidget);

    setCentralWidget(mainWidget);
}

void AtelierMainWindow::newFile(bool checked)
{
    // Open dialog to select a single file
    const QString fileName = QFileDialog::getOpenFileName(
        this,                        // Parent widget
        "Open File",                 // Dialog title
        "/home",                     // Starting directory
        "Text Files (*.csv)" // File filters
        );

    if (!fileName.isEmpty())
    {
        if(m_AtelierWidget)
        {
            m_AtelierWidget->init(sparsepc::openData<double>(fileName.toStdString(), ';'));

            m_MainWidgetStackedLayout->setCurrentWidget(m_AtelierWidget);
        }
    }
}

void AtelierMainWindow::open(bool checked)
{
    //m_MainWidgetStackedLayout->setCurrentWidget(m_AtelierWidget);
}

void AtelierMainWindow::save(bool checked)
{
    if(m_AtelierWidget)
    {
        m_AtelierWidget->save(checked);
    }
}

void AtelierMainWindow::close(bool checked)
{

}
