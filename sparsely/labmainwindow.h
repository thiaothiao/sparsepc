#pragma once

#include <memory>

#include <QMainWindow>

class QWidget;
class QCloseEvent;

namespace Sparsely
{
    class LabModel;
    class LabWidget;
    class LabController;

    class LabMainWindow final : public QMainWindow
    {
        Q_OBJECT

      public slots:
        bool newFile(bool checked);
        bool open(bool checked);
        bool save(bool checked);
        void close(bool checked) const;
        void showPreferences();
        void about();
        void aboutJKQTPlotter();
        void aboutEigen();

      public:
        LabMainWindow(QWidget *parent = nullptr);
        ~LabMainWindow();

      private:
        void closeEvent(QCloseEvent *event) override;
        std::unique_ptr<LabModel> m_LabModel;
        LabWidget *m_LabWidget;
        LabController *m_LabController;
    };
} // namespace Sparsely
