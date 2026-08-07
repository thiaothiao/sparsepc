#include <doctest/doctest.h>
#include <source/simu.hpp>
#include <sparsepc/core.hpp>
#include <sparsepc/infos.hpp>

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

TEST_CASE("Artificial data covariance")
{
    using Scalar = double;

    const auto sim = Sparsepc::linearmodel::generate_simulation<Scalar>(200, 1234);
    const auto sigma = sim.covariance();
    const auto theoretical = sim.theoretical_covariance();

    CHECK(sigma.rows() == 10);
    CHECK(sigma.cols() == 10);
    CHECK(theoretical.rows() == 10);
    CHECK(theoretical.cols() == 10);
    CHECK(theoretical(0, 0) == doctest::Approx(291.0));
    CHECK(theoretical(4, 4) == doctest::Approx(301.0));
    CHECK(theoretical(8, 8) == doctest::Approx(284.7875));
    CHECK(theoretical(0, 4) == doctest::Approx(0.0));
    CHECK(theoretical(0, 8) == doctest::Approx(-87.0));
    CHECK(theoretical(4, 8) == doctest::Approx(277.5));

    using Index = Sparsepc::Index;
    using Vector = Sparsepc::Vector<Scalar>;
    using Component = Sparsepc::Component<Scalar>;

    const auto n = sigma.cols();

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
    CHECK(std::abs(eigen.value - spectra.value) <= 1e-9);
    {
        const auto norm = (gram.vector - eigen.vector).norm();
        const auto ok = norm < 1e-5 && norm > 1e-6;
        CHECK(ok);
    }
    {
        const auto norm = (gram.vector - spectra.vector).norm();
        const auto ok = norm < 1e-5 && norm > 1e-6;
        CHECK(ok);
    }
    {
        const auto norm = (eigen.vector - spectra.vector).norm();
        const auto ok = norm < 1e-10;
        CHECK(ok);
    }

    const Index k0 = 4;
    const Index k1 = 4;

    {
        using BackwardGspca = Sparsepc::linearmodel::BackwardGspca<Scalar>;
        const BackwardGspca::Param param{{k0, k1}};
        const auto sparseEigenElements = BackwardGspca{param}.run(sigma);
        {
            //std::cout << "sparseEigenElements[0].value = " << std::fixed << std::setprecision(7) << sparseEigenElements[0].value << std::endl;
            CHECK(std::abs(sparseEigenElements[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.501963, 0.500532, 0.496671, 0.500818, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            //std::cout << "sparseEigenElements[1].value = " << std::fixed << std::setprecision(7) <<sparseEigenElements[1].value << std::endl;
            CHECK(std::abs(sparseEigenElements[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.496833, 0.502276, 0.501426, 0.499448, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using ForwardGspca = Sparsepc::linearmodel::ForwardGspca<Scalar>;
        const ForwardGspca::Param param{{k0, k1}};
        const auto sparseEigenElements = ForwardGspca{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 178569.2040900) < 1e-6);
            Vector v(n);
            v << 0, -0.4199159, 0, -0.4174335, 0, 0, 0, 0, 0.5718904, 0.5677687;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using ParallelGspca = Sparsepc::linearmodel::ParallelGspca<Scalar>;
        const ParallelGspca::Param param{{k0, k1}};
        const auto sparseEigenElements = ParallelGspca{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using Dca = Sparsepc::linearmodel::Dca<Scalar>;
        const Dca::Param param{{k0, k1}};
        const auto sparseEigenElements = Dca{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 284009.2987747) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5026805, 0.5016918, 0, 0, 0.4980111, 0.4975969;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using ContiguousFacetsFinder =  Sparsepc::linearmodel::ContiguousFacetsFinder<Scalar>;
        const ContiguousFacetsFinder::Param param{{k0, k1}};
        const auto sparseEigenElements = ContiguousFacetsFinder{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using MavIterativeElimination = Sparsepc::linearmodel::MavIterativeElimination<Scalar>;
        const MavIterativeElimination::Param param{{k0, k1}};
        const auto sparseEigenElements = MavIterativeElimination{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using AmvlIterativeElimination = Sparsepc::linearmodel::AmvlIterativeElimination<Scalar>;
        const AmvlIterativeElimination::Param param{{k0, k1}};
        const auto sparseEigenElements = AmvlIterativeElimination{param}.run(sigma);
        {
            CHECK(std::abs(sparseEigenElements[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(sparseEigenElements[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using BackwardGspca = Sparsepc::linearmodel::BackwardGspca<Scalar>;
        constexpr auto nbComponents = 2;
        const std::array<Index, nbComponents> choices{4, 4};
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
            CHECK(std::abs(validatedComponents[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.501963, 0.500532, 0.496671, 0.500818, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.496833, 0.502276, 0.501426, 0.499448, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using ForwardGspca = Sparsepc::linearmodel::ForwardGspca<Scalar>;
        constexpr auto nbComponents = 2;
        const std::array<Index, nbComponents> choices{4, 4};
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
            CHECK(std::abs(validatedComponents[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 178569.2040900) < 1e-6);
            Vector v(n);
            v << 0, -0.4199159, 0, -0.4174335, 0, 0, 0, 0, 0.5718904, 0.5677687;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using Dca = Sparsepc::linearmodel::Dca<Scalar>;
        constexpr auto nbComponents = 2;
        const std::array<Index, nbComponents> choices{4, 4};
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
            CHECK(std::abs(validatedComponents[0].value - 284009.2987747) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5026805, 0.5016918, 0, 0, 0.4980111, 0.4975969;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using ContiguousFacetsFinder =
            Sparsepc::linearmodel::ContiguousFacetsFinder<Scalar>;
        constexpr auto nbComponents = 2;
        const std::array<Index, nbComponents> choices{4, 4};
        std::vector<Component> validatedComponents;
        validatedComponents.reserve(nbComponents);
        for (Index j = 0; j < nbComponents; ++j)
        {
            auto candidates =
                ContiguousFacetsFinder::computeNextComponentCandidates(
                    sigma, ContiguousFacetsFinder::ImplementationParam{},
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
            CHECK(std::abs(validatedComponents[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using MavIterativeElimination =
            Sparsepc::linearmodel::MavIterativeElimination<Scalar>;
        constexpr auto nbComponents = 2;
        const std::array<Index, nbComponents> choices{4, 4};
        std::vector<Component> validatedComponents;
        validatedComponents.reserve(nbComponents);
        for (Index j = 0; j < nbComponents; ++j)
        {
            auto candidates =
                MavIterativeElimination::computeNextComponentCandidates(
                    sigma, MavIterativeElimination::ImplementationParam{},
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
            CHECK(std::abs(validatedComponents[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
    }

    {
        using AmvlIterativeElimination =
            Sparsepc::linearmodel::AmvlIterativeElimination<Scalar>;
        constexpr auto nbComponents = 2;
        const std::array<Index, nbComponents> choices{4, 4};
        std::vector<Component> validatedComponents;
        validatedComponents.reserve(nbComponents);
        for (Index j = 0; j < nbComponents; ++j)
        {
            auto candidates =
                AmvlIterativeElimination::computeNextComponentCandidates(
                    sigma, AmvlIterativeElimination::ImplementationParam{},
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
            CHECK(std::abs(validatedComponents[0].value - 290445.4200583) < 1e-6);
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() < 1e-6);
        }
        {
            CHECK(std::abs(validatedComponents[1].value - 226591.2637618) < 1e-6);
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() < 1e-6);
        }
    }
}

TEST_CASE("Sparsepc version")
{
    static_assert(Sparsepc::metadata::libVersion == std::string_view("0.2.0"));
    CHECK(std::string(Sparsepc::metadata::libVersion) == std::string("0.2.0"));
}
