#include "spcaloader.hpp"

#include <stdexcept>

#ifdef _WIN32

namespace
{
    /** @brief Windows version for dlerror(). */
    std::string dlerror()
    {
        DWORD const errorMessageID = GetLastError();
        if (errorMessageID == 0)
        {
            return std::string();
        }

        LPSTR messageBuffer = nullptr;
        size_t const size = FormatMessageA(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
              FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);

        LocalFree(messageBuffer);
        return message;
    }

    // Source - https://stackoverflow.com/a/38155689
    // Posted by Barmak Shemirani
    // Retrieved 2026-04-05, License - CC BY-SA 3.0
    std::wstring getUTF16(const std::string &str, int codepage = CP_UTF8)
    {
        if (str.empty())
        {
            return std::wstring();
        }
        int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
        std::wstring res(sz, 0);
        MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
        return res;
    }
} // namespace

#endif

namespace plugin
{
    SpcaLoader::SpcaLoader(std::string_view fileName) : libname(fileName)
    {
        // Open the library.
#ifdef _WIN32
        const auto wFileName  = getUTF16(libname);
        handle.reset(LoadLibrary(wFileName.c_str()));
#else
        handle.reset(dlopen(libname.c_str(), RTLD_LAZY));
#endif
        if (!handle)
        {
            throw std::runtime_error(dlerror());
        }

        /** @brief Pointer to the factory function. */
        using FactoryFn = SPCA* (*)();

          // Get the address of the figureFactory() function.
#ifdef _WIN32
        auto factory =
          reinterpret_cast<FactoryFn>(GetProcAddress(handle.get(), FACTORY_NAME));
#else
        auto factory = reinterpret_cast<FactoryFn>(dlsym(handle.get(), FACTORY_NAME));
#endif
        if (factory == nullptr)
        {
            throw std::runtime_error(dlerror());
        }

        // Create the figure instance.
        spca.reset(factory());
    }
} // namespace plugin
