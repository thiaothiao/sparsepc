#include <chrono>
#include <iostream>
#include <utility>

#include <../../test/source/simu.hpp>
#include <sparsepc/core.hpp>
#include <sparsepc/infos.hpp>

int main()
{
    std::cout << "Sparsepc library version " << Sparsepc::metadata::libVersion
              << "\n";

    using Scalar = double;
    using Matrix = Sparsepc::Matrix<Scalar>;
    using Vector = Sparsepc::Vector<Scalar>;
    using Component = Sparsepc::Component<Scalar>;
    using Index = Sparsepc::Index;

    const Index numberOfSamples = 200;
    const std::atomic_uint32_t seed = 1234;

    const auto sim = Sparsepc::linearmodel::generate_simulation<Scalar>(
        numberOfSamples, seed);
    const auto centeredX = sim.centered();
    {
        const Matrix sigma = centeredX.transpose() * centeredX;

        const auto tic = std::chrono::high_resolution_clock::now();
        auto gram = Sparsepc::EigenSolver<Scalar>{}.maximumValueElement(sigma);
        const auto toc = std::chrono::high_resolution_clock::now();
        auto eigen =
            Sparsepc::EigenLibEigenSolver<Scalar>{}.maximumValueElement(sigma);
        const auto tac = std::chrono::high_resolution_clock::now();
        auto spectra =
            Sparsepc::SpectraLibEigenSolver<Scalar>{}.maximumValueElement(
                sigma);
        const auto tuc = std::chrono::high_resolution_clock::now();

        std::cout << "\nGram method Maximum eigenvalue: " << gram.value << "\n"
                  << gram.vector.transpose() << "\n";
        std::cout << "\nEigen lib Maximum eigenvalue: " << eigen.value << "\n"
                  << eigen.vector.transpose() << "\n";
        std::cout << "\nSpectra lib Maximum eigenvalue: " << spectra.value
                  << "\n"
                  << spectra.vector.transpose() << "\n";

        std::cout << "\nGram duration: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(toc -
                                                                           tic)
                  << "\n";
        std::cout << "\nEigen duration: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(tac -
                                                                           toc)
                  << "\n";
        std::cout << "\nSpectra duration: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(tuc -
                                                                           tac)
                  << "\n";
    }

    const Index k0 = 4;
    const Index k1 = 4;

    {
        std::cout << "\nStarting backward run.\n";
        using BackwardGspca = Sparsepc::linearmodel::BackwardGspca<Scalar>;

        const auto start = std::chrono::high_resolution_clock::now();

        const BackwardGspca::Param param{{k0, k1}};

        const auto sparseEigenElements = BackwardGspca{param}.run(centeredX);

        const auto stop = std::chrono::high_resolution_clock::now();

        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nbackward run done in " << durationUs.count()
                  << " microseconds!\n";

        for (const auto &component : sparseEigenElements)
        {
            std::cout << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    {
        std::cout << "\nStarting forward run.\n";
        using ForwardGspca = Sparsepc::linearmodel::ForwardGspca<Scalar>;

        const auto start = std::chrono::high_resolution_clock::now();

        const ForwardGspca::Param param{{k0, k1}};

        const auto sparseEigenElements = ForwardGspca{param}.run(centeredX);

        const auto stop = std::chrono::high_resolution_clock::now();

        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nforward run done in " << durationUs.count()
                  << " microseconds!\n";

        for (const auto &component : sparseEigenElements)
        {
            std::cout << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    {
        std::cout << "\nStarting parallel run.\n";
        using ParallelGspca = Sparsepc::linearmodel::ParallelGspca<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const ParallelGspca::Param param{{k0, k1}};

        const auto sparseEigenElements = ParallelGspca{param}.run(centeredX);

        const auto stop = std::chrono::high_resolution_clock::now();

        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nparallel run done in " << durationUs.count()
                  << " microseconds!\n";

        for (const auto &component : sparseEigenElements)
        {
            std::cout << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    {
        std::cout << "\nStarting dca run.\n";
        using Dca = Sparsepc::linearmodel::Dca<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const Dca::Param param{{k0, k1}};

        const auto sparseEigenElements = Dca{param}.run(centeredX);

        const auto stop = std::chrono::high_resolution_clock::now();

        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\ndca run done in " << durationUs.count()
                  << " microseconds!\n";

        for (const auto &component : sparseEigenElements)
        {
            std::cout << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    {
        std::cout << "\nStarting contiguous facets finder run.\n";
        using ContiguousFacetsFinder =
            Sparsepc::linearmodel::ContiguousFacetsFinder<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const ContiguousFacetsFinder::Param param{{k0, k1}};

        const auto sparseEigenElements =
            ContiguousFacetsFinder{param}.run(centeredX);

        const auto stop = std::chrono::high_resolution_clock::now();

        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nContiguous facets finder run done in "
                  << durationUs.count() << " microseconds!\n";

        for (const auto &component : sparseEigenElements)
        {
            std::cout << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    {
        std::cout << "\nStarting iterative elimination algo. mav run.\n";
        using MavIterativeElimination =
            Sparsepc::linearmodel::MavIterativeElimination<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const MavIterativeElimination::Param param{{k0, k1}};

        const auto sparseEigenElements =
            MavIterativeElimination{param}.run(centeredX);

        const auto stop = std::chrono::high_resolution_clock::now();

        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nIterative elimination algo. mav run done in "
                  << durationUs.count() << " microseconds!\n";

        for (const auto &component : sparseEigenElements)
        {
            std::cout << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    {
        std::cout << "\nStarting iterative elimination algo. amvl run.\n";
        using AmvlIterativeElimination =
            Sparsepc::linearmodel::AmvlIterativeElimination<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const AmvlIterativeElimination::Param param{{k0, k1}};

        const auto sparseEigenElements =
            AmvlIterativeElimination{param}.run(centeredX);

        const auto stop = std::chrono::high_resolution_clock::now();

        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nIterative elimination algo. amvl run done in "
                  << durationUs.count() << " microseconds!\n";

        for (const auto &component : sparseEigenElements)
        {
            std::cout << component.value << ":\t"
                      << component.vector.transpose() << "\n";
        }
    }

    return 0;
}
