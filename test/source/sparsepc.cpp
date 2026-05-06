#include <doctest/doctest.h>
#include <source/simu.hpp>
#include <Sparsepc/core.hpp>
#include <Sparsepc/infos.hpp>

#include <cstdlib>
#include <string>

namespace
{
    template <std::floating_point ScalarType>
    inline ScalarType sign(ScalarType value)
    {
        return std::signbit(value) ? static_cast<ScalarType>(-1)
                                   : static_cast<ScalarType>(1);
    }
} // namespace

TEST_CASE("Sparsepc all")
{
    using Scalar = double;
    using Index = Sparsepc::Index;
    using Vector = Sparsepc::Vector<Scalar>;
    using Component = Sparsepc::Component<Scalar>;

    const auto sigma = Sparsepc::linearmodel::pitprops<Scalar>();
    const auto n = sigma.cols();
    CHECK(n == 13);

    auto gram = Sparsepc::EigenSolver<Scalar>{}.maximumValueElement(sigma);
    auto eigen =
        Sparsepc::EigenLibEigenSolver<Scalar>{}.maximumValueElement(sigma);
    auto spectra =
        Sparsepc::SpectraLibEigenSolver<Scalar>{}.maximumValueElement(sigma);

    Index idxMaxCoeff = 0;
    gram.vector.cwiseAbs().maxCoeff(&idxMaxCoeff);
    gram.vector *= sign(gram.vector[idxMaxCoeff]);
    eigen.vector.cwiseAbs().maxCoeff(&idxMaxCoeff);
    eigen.vector *= sign(eigen.vector[idxMaxCoeff]);
    spectra.vector.cwiseAbs().maxCoeff(&idxMaxCoeff);
    spectra.vector *= sign(spectra.vector[idxMaxCoeff]);

    CHECK(std::abs(gram.value - eigen.value) <= 1e-4);
    CHECK(std::abs(gram.value - spectra.value) <= 1e-4);
    CHECK(std::abs(eigen.value - spectra.value) <= 1e-10);
    {
        const auto norm = (gram.vector - eigen.vector).norm();
        const auto ok = norm < 1e-2 && norm > 1e-3;
        CHECK(ok);
    }
    {
        const auto norm = (gram.vector - spectra.vector).norm();
        const auto ok = norm < 1e-2 && norm > 1e-3;
        CHECK(ok);
    }
    {
        const auto norm = (eigen.vector - spectra.vector).norm();
        const auto ok = norm < 1e-10;
        CHECK(ok);
    }

    const Index k0 = 6;
    const Index k1 = 2;
    const Index k2 = 2;

    {
        using BackwardGspca = Sparsepc::linearmodel::BackwardGspca<Scalar>;
        const BackwardGspca::Param param{{k0, k1, k2}};
        const auto sparseEigenElements = BackwardGspca{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 3.77096) < 1e-4);
            Vector v(n);
            v << 0.444022, 0.453036, 0, 0, 0, 0, 0.378041, 0.341986, 0.403168,
                0.418555, 0, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 1.882) < 1e-4);
            Vector v(n);
            v << 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[2].value - 1.364) < 1e-4);
            Vector v(n);
            v << 0, 0, 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[2].vector - v).norm() < 1e-6);
        }
    }

    {
        using ForwardGspca = Sparsepc::linearmodel::ForwardGspca<Scalar>;
        const ForwardGspca::Param param{{k0, k1, k2}};
        const auto sparseEigenElements = ForwardGspca{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 3.77096) < 1e-4);
            Vector v(n);
            v << 0.444022, 0.453036, 0, 0, 0, 0, 0.378041, 0.341986, 0.403168,
                0.418555, 0, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 1.882) < 1e-4);
            Vector v(n);
            v << 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[2].value - 1.364) < 1e-4);
            Vector v(n);
            v << 0, 0, 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[2].vector - v).norm() < 1e-6);
        }
    }

    {
        using ParallelGspca = Sparsepc::linearmodel::ParallelGspca<Scalar>;
        const ParallelGspca::Param param{{k0, k1, k2}};
        const auto sparseEigenElements = ParallelGspca{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 3.77096) < 1e-4);
            Vector v(n);
            v << 0.444022, 0.453036, 0, 0, 0, 0, 0.378041, 0.341986, 0.403168,
                0.418555, 0, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 1.882) < 1e-4);
            Vector v(n);
            v << 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[2].value - 1.364) < 1e-4);
            Vector v(n);
            v << 0, 0, 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[2].vector - v).norm() < 1e-6);
        }
    }

    {
        using Dca = Sparsepc::linearmodel::Dca<Scalar>;
        const Dca::Param param{{k0, k1, k2}};
        const auto sparseEigenElements = Dca{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 3.77096) < 1e-4);
            Vector v(n);
            v << 0.444342, 0.45335, 0, 0, 0, 0, 0.378054, 0.34141, 0.403122,
                0.418378, 0, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 1.882) < 1e-4);
            Vector v(n);
            v << 0, 0, 0.707168, 0.707045, 0, 0, 0, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[2].value - 1.32898) < 1e-4);
            Vector v(n);
            v << 0, 0, 0, 0, 0, 0.711242, 0, 0, 0, 0, 0, 0, -0.702948;
            CHECK((sparseEigenElements[2].vector - v).norm() < 1e-6);
        }
    }

    {
        using BackwardGspca = Sparsepc::linearmodel::BackwardGspca<Scalar>;
        constexpr auto nbComponents = 3;
        const std::array<Index, nbComponents> choices{6, 2, 2};
        std::vector<Component> validatedComponents;
        validatedComponents.reserve(nbComponents);
        for (Index j = 0; j < nbComponents; ++j)
        {
            auto candidates = BackwardGspca::computeNextComponentCandidates(
                sigma, BackwardGspca::ImplementationParam{},
                validatedComponents, nullptr);
            const auto iCandidate = choices[j];
            for (auto &[i, candidate] : candidates)
            {
                candidate.state = Sparsepc::ComponentState::Unvalidated;
            }
            candidates.at(iCandidate).state =
                Sparsepc::ComponentState::Validated;
            validatedComponents.push_back(std::move(candidates.at(iCandidate)));
        }
        {
            CHECK(std::abs(validatedComponents[0].value - 3.77096) < 1e-4);
            Vector v(n);
            v << 0.444022, 0.453036, 0, 0, 0, 0, 0.378041, 0.341986, 0.403168,
                0.418555, 0, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 1.882) < 1e-4);
            Vector v(n);
            v << 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[2].value - 1.364) < 1e-4);
            Vector v(n);
            v << 0, 0, 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[2].vector - v).norm() < 1e-6);
        }
    }

    {
        using ForwardGspca = Sparsepc::linearmodel::ForwardGspca<Scalar>;
        constexpr auto nbComponents = 3;
        const std::array<Index, nbComponents> choices{6, 2, 2};
        std::vector<Component> validatedComponents;
        validatedComponents.reserve(nbComponents);
        for (Index j = 0; j < nbComponents; ++j)
        {
            auto candidates = ForwardGspca::computeNextComponentCandidates(
                sigma, ForwardGspca::ImplementationParam{}, validatedComponents,
                nullptr);
            const auto iCandidate = choices[j];
            for (auto &[i, candidate] : candidates)
            {
                candidate.state = Sparsepc::ComponentState::Unvalidated;
            }
            candidates.at(iCandidate).state =
                Sparsepc::ComponentState::Validated;
            validatedComponents.push_back(std::move(candidates.at(iCandidate)));
        }

        {
            CHECK(std::abs(validatedComponents[0].value - 3.77096) < 1e-4);
            Vector v(n);
            v << 0.444022, 0.453036, 0, 0, 0, 0, 0.378041, 0.341986, 0.403168,
                0.418555, 0, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 1.882) < 1e-4);
            Vector v(n);
            v << 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[2].value - 1.364) < 1e-4);
            Vector v(n);
            v << 0, 0, 0, 0, 0.707107, 0.707107, 0, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[2].vector - v).norm() < 1e-6);
        }
    }

    {
        using Dca = Sparsepc::linearmodel::Dca<Scalar>;
        constexpr auto nbComponents = 3;
        const std::array<Index, nbComponents> choices{6, 2, 2};
        std::vector<Component> validatedComponents;
        validatedComponents.reserve(nbComponents);
        for (Index j = 0; j < nbComponents; ++j)
        {
            auto candidates = Dca::computeNextComponentCandidates(
                sigma, Dca::ImplementationParam{}, validatedComponents,
                nullptr);
            const auto iCandidate = choices[j];
            for (auto &[i, candidate] : candidates)
            {
                candidate.state = Sparsepc::ComponentState::Unvalidated;
            }
            candidates.at(iCandidate).state =
                Sparsepc::ComponentState::Validated;
            validatedComponents.push_back(std::move(candidates.at(iCandidate)));
        }
        {
            CHECK(std::abs(validatedComponents[0].value - 3.77096) < 1e-4);
            Vector v(n);
            v << 0.444342, 0.45335, 0, 0, 0, 0, 0.378054, 0.34141, 0.403122,
                0.418378, 0, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 1.882) < 1e-4);
            Vector v(n);
            v << 0, 0, 0.707168, 0.707045, 0, 0, 0, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[2].value - 1.32898) < 1e-4);
            Vector v(n);
            v << 0, 0, 0, 0, 0, 0.711242, 0, 0, 0, 0, 0, 0, -0.702948;
            CHECK((validatedComponents[2].vector - v).norm() < 1e-6);
        }
    }
}

TEST_CASE("Sparsepc version")
{
    static_assert(Sparsepc::metadata::libVersion == std::string_view("0.2.0"));
    CHECK(std::string(Sparsepc::metadata::libVersion) == std::string("0.2.0"));
}
