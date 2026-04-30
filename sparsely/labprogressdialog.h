#pragma once

#include <functional>

class QProgressDialog;

namespace sparsely
{
    class ProgressDialog
    { // progress dialog wrapper used during computations
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
