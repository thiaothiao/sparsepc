#pragma once

#include <concepts>
#include <utility>

namespace Sparsepc
{
    /**
     * @brief Concept for a type that can be used to report progresses.
     * @details This concept ensures the type supports
     * @param setRange a member function that sets the min and max values.
     * @param setValue a member function that sets current progress value.
     * @param wasCanceled a const member function that returns true if cancelled
     * and false otherwise.
     * @param processEvents a const member function that allows to process
     * events as needed.
     */
    template <class ImplementationType>
    concept ProgressBarLike = requires(ImplementationType impl) {
        { impl.setValue(int{}) };
        { impl.setRange(int{}, int{}) };
        { std::as_const(impl).wasCanceled() } -> std::convertible_to<bool>;
        { std::as_const(impl).processEvents() };
    };

    /**
     * @brief A dummy progress bar class to be used by default.
     * @details It does almost nothing.
     */
    class DummyProgressBar final
    {
      public:
        DummyProgressBar() {}
        void setValue([[maybe_unused]] int value) {}
        void setRange([[maybe_unused]] int minValue,
                      [[maybe_unused]] int maxValue)
        {
        }
        bool wasCanceled() const { return false; }
        void processEvents() const {}
    };
} // namespace Sparsepc
