#include "labprogressdialog.h"

#include <QApplication>
#include <QProgressDialog>

namespace sparsely
{
    ProgressDialog::ProgressDialog(QProgressDialog &aQProgressDialog)
        : qProgressDialog{aQProgressDialog}
    {
    }

    void ProgressDialog::setValue(int value)
    {
        qProgressDialog.get().setValue(value);
    }

    void ProgressDialog::setRange(int minValue, int maxValue)
    {
        qProgressDialog.get().setRange(minValue, maxValue);
    }

    bool ProgressDialog::wasCanceled() const
    {
        return qProgressDialog.get().wasCanceled();
    }

    void ProgressDialog::processEvents() const
    {
        // QCoreApplication::processEvents();
    }
} // namespace sparsely
