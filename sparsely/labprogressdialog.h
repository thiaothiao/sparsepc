#ifndef SPARSEPC_PROGRESS_DIALOG_HPP
#define SPARSEPC_PROGRESS_DIALOG_HPP

#include <functional>

class QProgressDialog;

namespace sparsely
{
    class ProgressDialog
    { // progress dialog wrapper
      public:
        ProgressDialog(QProgressDialog &aQProgressDialog);

        void setValue(int value);
        void setRange(int minValue, int maxValue);
        bool wasCanceled() const;
        void processEvents() const;

      private:
        std::reference_wrapper<QProgressDialog> qProgressDialog;
    };
} // namespace sparsely
#endif // SPARSEPC_PROGRESS_DIALOG_HPP
