#ifndef SPARSEPC_PROGRESSBAR_HPP
#define SPARSEPC_PROGRESSBAR_HPP

namespace sparsepc
{
    template <class ImplementationType>
    concept ProgressBarLike = requires(ImplementationType impl)
    {
        {
            impl.setValue(int{})
        };

        {
            impl.setRange(int{}, int{})
        };
    };

    class DummyProgressBar final
    {
    public:

        DummyProgressBar()
        {
        }

        void setValue([[maybe_unused]] int value) {}

        void setRange([[maybe_unused]] int minValue, [[maybe_unused]] int maxValue){}
    };
}
#endif //SPARSEPC_PROGRESSBAR_HPP
