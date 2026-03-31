#include <QApplication>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QWidget *window = new QWidget;
    window->resize(400, 300);

    QProgressBar *progressBar = new QProgressBar(window);
    progressBar->setRange(0, 100);
    progressBar->setValue(50);
    progressBar->setGeometry(50, 120, 300, 50); // Position it as an overlay

    // 1. Make the QProgressBar background itself transparent via stylesheet
    progressBar->setStyleSheet(
        "QProgressBar { background-color: transparent; border: 1px solid grey; } QProgressBar::chunk { background: blue; }");

    // 2. Apply a QGraphicsOpacityEffect to control the widget's overall semi-transparency
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(progressBar);
    opacityEffect->setOpacity(0.5); // Set desired opacity (0.0 to 1.0)
    progressBar->setGraphicsEffect(opacityEffect);

    // Optionally, use a semi-transparent color for the chunk itself via stylesheet if preferred
    // progressBar->setStyleSheet("QProgressBar::chunk { background: rgba(0, 0, 255, 128); }"); // 128 is 50% alpha


    window->show();

    return a.exec();
}
