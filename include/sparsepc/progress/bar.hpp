#ifndef SPARSEPC_PROGRESSBAR_HPP
#define SPARSEPC_PROGRESSBAR_HPP

#include <concepts>
#include <utility>

namespace sparsepc
{
    template <class ImplementationType>
    concept ProgressBarLike = requires(ImplementationType impl) {
        { impl.setValue(int{}) };
        { impl.setRange(int{}, int{}) };
        { std::as_const(impl).wasCanceled() } -> std::convertible_to<bool>;
        { std::as_const(impl).processEvents() };
    };

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
} // namespace sparsepc
#endif // SPARSEPC_PROGRESSBAR_HPP
