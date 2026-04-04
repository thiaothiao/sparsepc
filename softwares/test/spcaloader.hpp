#ifndef SPCALOADER_HPP
#define SPCALOADER_HPP

#include <memory>
#include <string>
#include <string_view>

#include "spca.hpp"

#ifdef _WIN32
    #include <windows.h>
    #include <type_traits>

    // Just for using the same name in both platforms.
    using LibraryHandler = std::remove_pointer<HMODULE>::type;

    inline void dlclose(LibraryHandler* handle) noexcept
    {
        FreeLibrary(handle);
    }
#else
    #include <dlfcn.h>
    using LibraryHandler = void;
#endif

namespace plugin
{
    /**
     * @brief Loads and holds a spca from a shared library.
     */
    class SpcaLoader
    {
    public:
        /**
        * @brief Loads an instance of a plugin::Figure.
        * @param libraryPath Name of the file that contains the figure.
        * @throw std::runtime_error In case of error.
        */
        explicit SpcaLoader(std::string_view libraryPath);

        /** @brief Gets the name of the file loaded. */
        std::string const& getLibname() const noexcept
        {
            return libname;
        }

        /** @brief Gets the instance of the figure loaded. */
        SPCA& getSpca() const& noexcept
        {
            return *spca;
        }

        /** @note Not for rvalues. The library should remain open. */
        void getSpca() && = delete;

    private:
        /** @brief Deleter for the library handler. */
        struct LibHandlerDeleter
        {
            void operator()(LibraryHandler* handle) noexcept { dlclose(handle); }
        };

        std::string libname;
        std::unique_ptr<LibraryHandler, LibHandlerDeleter> handle;
        std::unique_ptr<SPCA> spca;
    };

} // namespace plugin
#endif // SPCALOADER_HPP
