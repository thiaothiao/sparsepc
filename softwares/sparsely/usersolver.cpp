#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <limits>
#include <algorithm>
#include <future>
#include <concepts>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

#include "usersolver.hpp"
#include "spcaloader.hpp"

#include "sparsepc/utils/matrix.hpp"
#include "sparsepc/eigen/solver.hpp"
#include "sparsepc/generic/solver.hpp"
#include "sparsepc/progress/bar.hpp"

namespace
{
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
}

namespace sparsepc
{
    namespace linearmodel
    {
        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto UserModel<ScalarType, EigenSolverType, ProgressBarType>::run(
            const Matrix<Scalar>& sigma) const
        {
            using Component = Component<Scalar>;

            const auto k = m_Param.k;
            const auto& eigenSolver = m_Param.eigenSolver;

            const auto n = sigma.cols();
            if (k >= n || k < static_cast<Index>(0))
            {
                return eigenSolver.maximumValueElement(sigma);
            }

            if (static_cast<Index>(1) == n)
            {
                Component cmponent(n);
                cmponent.value = sigma.value();
                cmponent.vector.setOnes();

                return cmponent;
            }

            Component component(n);

            std::string const pluginsDir = [&]() {
                //if (argc > 1)
                // {
                //    return std::string(argv[1]);
                //}
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

                spca.solve(sigma.data(), n, k, component.vector.data());

                component.vector.normalize();

                component.value = (component.vector.transpose() * sigma * component.vector).value();

                std::cout << "\n Job done.\n";
            }
            catch (std::out_of_range const&)
            {
                std::cout << "\n Wrong.\n";
            }

            return component;
        }

        template<std::floating_point ScalarType, EigenSolverLike EigenSolverType, ProgressBarLike ProgressBarType>
        auto UserModel<ScalarType, EigenSolverType, ProgressBarType>::runAll(const Matrix<Scalar>& sigma,
                                                                             const Param& param, ProgressBar* progressBar)
        {
            using Component = Component<Scalar>;
            using ComponentsContainer = ComponentsContainer<Component>;

            const auto& eigenSolver = param.eigenSolver;
            const auto n = sigma.cols();

            ComponentsContainer components;

            const auto& component = components.emplace(n, eigenSolver.maximumValueElement(sigma)).first->second;

            if (n == 1)
            {
                return components;
            }

            for (Index k = 1; k < n; ++k)
            {
                components.emplace(k, Component{});
            }

            /*********************************/

            std::string const pluginsDir = [&]() {
                //if (argc > 1)
               // {
                //    return std::string(argv[1]);
                //}
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

                for (Index k = 1; k < n; ++k)
                {
                    if(progressBar)
                    {
                        progressBar->setValue(k);
                    }

                    auto& component = components.at(k);

                    spca.solve(sigma.data(), n, k, component.vector.data());

                    component.vector.normalize();

                    component.value = (component.vector.transpose() * sigma * component.vector).value();
                }

                std::cout << "\n Job done.\n";
            }
            catch (std::out_of_range const&)
            {
                std::cout << "\n Wrong.\n";
            }

            return components;
        }

        //template<std::floating_point ScalarType>
        //using User = SparsePC<UserModel<ScalarType, EigenSolver<ScalarType>>>;
    }
}
