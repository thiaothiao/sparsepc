#include <chrono>
#include <iostream>
#include <utility>

#include <../../test/source/simu.hpp>
#include <sparsepc/core.hpp>
#include <sparsepc/infos.hpp>

int main()
{
    std::cout << "sparsepc library version " << sparsepc::metadata::libVersion
              << "\n";

    using Scalar = double;
    using Matrix = sparsepc::Matrix<Scalar>;
    using Vector = sparsepc::Vector<Scalar>;
    using Component = sparsepc::Component<Scalar>;
    using Index = sparsepc::Index;

    const auto sigma = sparsepc::linearmodel::pitprops<Scalar>();

    const auto tic = std::chrono::high_resolution_clock::now();
    auto gram = sparsepc::EigenSolver<Scalar>{}.maximumValueElement(sigma);
    const auto toc = std::chrono::high_resolution_clock::now();
    auto eigen =
        sparsepc::EigenLibEigenSolver<Scalar>{}.maximumValueElement(sigma);
    const auto tac = std::chrono::high_resolution_clock::now();
    auto spectra =
        sparsepc::SpectraLibEigenSolver<Scalar>{}.maximumValueElement(sigma);
    const auto tuc = std::chrono::high_resolution_clock::now();

    std::cout << "\nGram method Maximum eigenvalue: " << gram.value << "\n"
              << gram.vector.transpose() << "\n";
    std::cout << "\nEigen lib Maximum eigenvalue: " << eigen.value << "\n"
              << eigen.vector.transpose() << "\n";
    std::cout << "\nSpectra lib Maximum eigenvalue: " << spectra.value << "\n"
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

    const Index k0 = 6;
    const Index k1 = 2;
    const Index k2 = 2;

    {
        std::cout << "\nStarting backward run.\n";
        using BackwardGspca = sparsepc::linearmodel::BackwardGspca<Scalar>;

        const auto start = std::chrono::high_resolution_clock::now();

        const BackwardGspca::Param param{{k0, k1, k2}};

        const auto sparseEigenElements = BackwardGspca{param}.run(sigma);

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
        using ForwardGspca = sparsepc::linearmodel::ForwardGspca<Scalar>;

        const auto start = std::chrono::high_resolution_clock::now();

        const ForwardGspca::Param param{{k0, k1, k2}};

        const auto sparseEigenElements = ForwardGspca{param}.run(sigma);

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
        using ParallelGspca = sparsepc::linearmodel::ParallelGspca<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const ParallelGspca::Param param{{k0, k1, k2}};

        const auto sparseEigenElements = ParallelGspca{param}.run(sigma);

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
        using Dca = sparsepc::linearmodel::Dca<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const Dca::Param param{{k0, k1, k2}};

        const auto sparseEigenElements = Dca{param}.run(sigma);

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

    return 0;
}
