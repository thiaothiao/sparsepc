#ifndef SPCA_HPP
#define SPCA_HPP

#include <string>
#include <string_view>

namespace plugin
{
    /** @brief Name of the factory function that creates a spca object. */
    constexpr auto FACTORY_NAME = "spcaFactory";

    /**
     * @brief Abstract class that represents a plugin Figure.
     * @details Every plugin in this application should extends this class and
     * override the functions userInput(), area() and perimeter(). It also requires
     * to define a C function spcaFactory() that returns a pointer to a
     * Figure object.
     * @note MSVC doesn't support std::unique_ptr as return type in C functions, so
     * a raw pointer is used.
     */
    class SPCA
    {
    public:
        SPCA(std::string_view spcaName) : name(spcaName) {}

        virtual ~SPCA() = default;

        std::string const& getName() const noexcept { return name; }

        virtual void solve(const double* sigmaData,int n, int k, double* solution) const = 0;

    protected:
        SPCA(SPCA const& other) = delete;
        SPCA(SPCA&& other) noexcept = default;
        SPCA& operator=(SPCA const& other) = delete;
        SPCA& operator=(SPCA&& other) noexcept = default;

    private:
        std::string name;
    };

} // namespace plugin

#ifdef _MSC_VER
// Required for MSVC to locate the functions correctly.
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

/** @brief Default implementation of the spcaFactory() function. */
#define DEFAULT_SPCA_FACTORY(spca)                \
  extern "C" EXPORT plugin::SPCA* spcaFactory() { \
    return new spca();                              \
  }

#endif // SPCA_HPP
