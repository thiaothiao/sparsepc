#include <iostream>
#include <chrono>
#include <utility>
#include <vector>

#include <Eigen/Dense>

#include "sparsepc/version.hpp"
#include "sparsepc/core.hpp"
#include "simu.hpp"

int main()
{
    constexpr std::string_view version = SPARSEPC_MACRO_STRINGIFY(SPARSEPC_VERSION);
    std::cout << "sparsepc library version " << version << "\n";

    using Scalar = double;
    using Matrix = sparsepc::Matrix<Scalar>;
    using Vector = sparsepc::Vector<Scalar>;
    using Index = sparsepc::Index;

    const auto sigma = sparsepc::linearmodel::pitprops<Scalar>();
    const auto n = sigma.cols();

    const auto tic = std::chrono::high_resolution_clock::now();
    auto eeGram = sparsepc::EigenSolver<Scalar>{}.maximumValueElement(sigma);
    const auto toc = std::chrono::high_resolution_clock::now();
    auto eeEigen = sparsepc::EigenLibEigenSolver<Scalar>{}.maximumValueElement(sigma);
    const auto tac = std::chrono::high_resolution_clock::now();

    std::cout << "\nGram Maximum eigenvalue: "
        << eeGram.value << "\n"
        << eeGram.vector.transpose() << "\n";
    std::cout << "\nEigen Maximum eigenvalue: "
        << eeEigen.value << "\n"
        << eeEigen.vector.transpose() << "\n";

    // Calculate the duration and cast to microseconds
    std::cout << "\nGram duration: "
        << std::chrono::duration_cast<std::chrono::microseconds>(toc - tic) << "\n";
    std::cout << "\nEigen duration: "
        << std::chrono::duration_cast<std::chrono::microseconds>(tac - toc) << "\n";

    const Index k0 = 6;
    const Index k1 = 2;
    const Index k2 = 2;   

    {
        std::cout << "\nStarting backward run.\n";
        using BackwardGspca = sparsepc::linearmodel::BackwardGspca<Scalar>;

        const auto start = std::chrono::high_resolution_clock::now();

        const BackwardGspca::Param param{ {k0, k1, k2} };

        const auto sparseEigenElements = BackwardGspca{ param }.run(sigma);

        const auto stop = std::chrono::high_resolution_clock::now();

        // Calculate the duration and cast to microseconds
        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nbackward run done in "
            << durationUs.count() << " microseconds!\n";

        std::cout << sparsepc::toMatrix<Scalar>(sparseEigenElements) << "\n";
    }

    {
        std::cout << "\nStarting forward run.\n";
        using ForwardGspca = sparsepc::linearmodel::ForwardGspca<Scalar>;

        const auto start = std::chrono::high_resolution_clock::now();

        const ForwardGspca::Param param{ {k0, k1, k2} };

        const auto sparseEigenElements = ForwardGspca{ param }.run(sigma);

        const auto stop = std::chrono::high_resolution_clock::now();

        // Calculate the duration and cast to microseconds
        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nforward run done in "
            << durationUs.count() << " microseconds!\n";

        std::cout << sparsepc::toMatrix<Scalar>(sparseEigenElements) << "\n";
    }

    {
        std::cout << "\nStarting parallel run.\n";
        using ParallelGspca = sparsepc::linearmodel::ParallelGspca<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const ParallelGspca::Param param{ {k0, k1, k2} };

        const auto sparseEigenElements = ParallelGspca{ param }.run(sigma);

        const auto stop = std::chrono::high_resolution_clock::now();

        // Calculate the duration and cast to microseconds
        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\nparallel run done in "
            << durationUs.count() << " microseconds!\n";

        std::cout << sparsepc::toMatrix<Scalar>(sparseEigenElements) << "\n";
    }

    {
        std::cout << "\nStarting dca run.\n";
        using Dca = sparsepc::linearmodel::Dca<Scalar>;
        const auto start = std::chrono::high_resolution_clock::now();

        const Dca::Param param{ {k0, k1, k2} };

        const auto sparseEigenElements = Dca{ param }.run(sigma);

        const auto stop = std::chrono::high_resolution_clock::now();

        // Calculate the duration and cast to microseconds
        const auto durationUs =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n\ndca run done in "
            << durationUs.count() << " microseconds!\n";

        std::cout << sparsepc::toMatrix<Scalar>(sparseEigenElements) << "\n";
    }

    /*******************************************************************************/
    {
        std::cout << "\nStarting spcs validation: dca run.\n";
        using Dca = sparsepc::linearmodel::Dca<Scalar>;
        using Component = sparsepc::Component<Scalar>;
        using ComponentsContainer = std::vector<Component>;

        const Dca::Param param{ {k0, k1, k2} };

        ComponentsContainer validatedComponents;
        validatedComponents.reserve(param.nbComponents);

        for (Index j = 0; j < param.nbComponents; ++j)
        {
            auto candidates = Dca::computeNextComponentCandidates(
                sigma, param.modelParams[j], validatedComponents);

            std::cout << sparsepc::toMatrix<Scalar>(candidates) << "\n";

            auto iCandidate = static_cast<Index>(0);
            std::cout << "Choose one candidate\n";
            std::cin >> iCandidate;
            std::cout << "\n";

            for (auto& candidate : candidates)
            {
                candidate.state = sparsepc::ComponentState::Unvalidated;
            }
            candidates[iCandidate].state = sparsepc::ComponentState::Validated;

            validatedComponents.push_back(std::move(candidates[iCandidate]));
        }

        std::cout << sparsepc::toMatrix<Scalar>(validatedComponents) << "\n";
    }

    return 0;
    {
        std::cout << "\nStarting spcs validation: forward run.\n";
        using ForwardGspca = sparsepc::linearmodel::ForwardGspca<Scalar>;
        using Component = sparsepc::Component<Scalar>;
        using ComponentsContainer = std::vector<Component>;

        const ForwardGspca::Param param{ {k0, k1, k2} };

        ComponentsContainer validatedComponents;
        validatedComponents.reserve(param.nbComponents);

        for (Index j = 0; j < param.nbComponents; ++j)
        {
            auto candidates = ForwardGspca::computeNextComponentCandidates(
                sigma, param.modelParams[j], validatedComponents);

            std::cout << sparsepc::toMatrix<Scalar>(candidates) << "\n";

            auto iCandidate = static_cast<Index>(0);
            std::cout << "Choose one candidate\n";
            std::cin >> iCandidate;
            std::cout << "\n";

            for (auto& candidate : candidates)
            {
                candidate.state = sparsepc::ComponentState::Unvalidated;
            }
            candidates[iCandidate].state = sparsepc::ComponentState::Validated;

            validatedComponents.push_back(std::move(candidates[iCandidate]));
        }

        std::cout << sparsepc::toMatrix<Scalar>(validatedComponents) << "\n";
    }

    return 0;

    {
        std::cout << "\nStarting spcs validation: backward run.\n";
        using BackwardGspca = sparsepc::linearmodel::BackwardGspca<Scalar>;
        using Component = sparsepc::Component<Scalar>;
        using ComponentsContainer = std::vector<Component>;
        
        const BackwardGspca::Param param{ {k0, k1, k2} };

        ComponentsContainer validatedComponents;
        validatedComponents.reserve(param.nbComponents);

        for (Index j = 0; j < param.nbComponents; ++j)
        {
            auto candidates = BackwardGspca::computeNextComponentCandidates(
                sigma, param.modelParams[j], validatedComponents);

            std::cout << sparsepc::toMatrix<Scalar>(candidates) << "\n";

            auto iCandidate = static_cast<Index>(0);
            std::cout << "Choose one candidate\n";
            std::cin >> iCandidate;
            std::cout << "\n";

            for (auto& candidate: candidates)
            {
                candidate.state = sparsepc::ComponentState::Unvalidated;
            }
            candidates[iCandidate].state = sparsepc::ComponentState::Validated;

            validatedComponents.push_back(std::move(candidates[iCandidate]));
        }

        std::cout << sparsepc::toMatrix<Scalar>(validatedComponents) << "\n";
    }

    return 0;
}