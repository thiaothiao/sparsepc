#include "sparsepc/version.hpp"

#include "AtelierMainWindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>

#include "simu.hpp"
#include <Eigen/Dense>

AtelierMainWindow::AtelierMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    constexpr std::string_view version = SPARSEPC_MACRO_STRINGIFY(SPARSEPC_VERSION);
    this->setWindowTitle("SPARSELY Software - SPARSE principal component LaboratorY");
    this->setMinimumSize(640, 500);

     auto* newAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew),
                          tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    QObject::connect(newAct, &QAction::triggered, this, &AtelierMainWindow::newFile);

    auto* openAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
                          tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    QObject::connect(openAct, &QAction::triggered, this, &AtelierMainWindow::open);

    auto* quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit"));
    QObject::connect(quitAct, &QAction::triggered, this, &AtelierMainWindow::close);

    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    //fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

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
        m_AtelierWidget->init(sparsepc::openData<double>(fileName.toStdString(), ';'));

        m_MainWidgetStackedLayout->setCurrentWidget(m_AtelierWidget);
    }
}

void AtelierMainWindow::open(bool checked)
{
    //m_MainWidgetStackedLayout->setCurrentWidget(m_AtelierWidget);
}

void AtelierMainWindow::close(bool checked)
{

}
