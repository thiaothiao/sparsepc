#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "spcaloader.hpp"

/**
 * @brief Get the list of files in a directory.
 * @param directory Name of the directory.
 * @return A vector with the names of the files in the directory.
 */
std::vector<std::string> getPluginList(std::string_view directory);

int main(int argc, char** argv)
{
    std::string const pluginsDir = [&]() {
                if (argc > 1)
                {
                    return std::string(argv[1]);
                }
                return std::string("plugins");
          }();

  std::vector<plugin::SpcaLoader> loaders;

    for (auto const plugins = getPluginList(pluginsDir);
       auto const& pluginFile : plugins)
    {
        try
        {
            std::cout << "Loading " << pluginFile << "...";
            loaders.emplace_back(pluginFile);
            std::cout << " Loaded!\n";
        }
        catch (std::runtime_error const& e)
        {
            std::cerr << "Failed: " << e.what() << '\n';
        }
    }

    std::cout << "\n loaded plugins \n";
    for (int index = 1; auto const& loader : loaders)
    {
        std::cout << "\n\t" << index++ << ") " << loader.getSpca().getName();
    }
    std::cout << "\n plugin loaded.\n";

    try
    {
        const plugin::SPCA& spca = loaders.at(0).getSpca();

        spca.solve(nullptr, 0, 0, nullptr);

        std::cout << "\n Job done.\n";
    }
    catch (std::out_of_range const&)
    {
        std::cout << "\n Wrong.\n";
    }

    return 0;
}

std::vector<std::string> getPluginList(std::string_view directory)
try
{
    std::filesystem::path const pluginsDir(directory);
    std::vector<std::string> plugins;

    for (auto const& entry : std::filesystem::directory_iterator(pluginsDir))
    {
        if (auto const& ext = entry.path().extension();
            entry.is_regular_file() &&
            (ext == ".dll" || ext == ".so" || ext == ".dylib"))
        {
            plugins.push_back(entry.path().relative_path().string());
        }
    }
    return plugins;
}
catch (std::filesystem::filesystem_error const& e)
{
    std::cerr << e.what() << '\n';
    return {};
}
