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
    using Index = Sparsepc::Index;
    using Vector = Sparsepc::Vector<Scalar>;
    using Component = Sparsepc::Component<Scalar>;

    const Index numberOfSamples = 200;
    const std::uint32_t seed = 1234;
    const auto sim = Sparsepc::linearmodel::generate_simulation<Scalar>(
        numberOfSamples, seed);
    const auto &centeredX = sim.centered();
    const auto theoretical = sim.theoretical_covariance();

    CHECK(centeredX.rows() == numberOfSamples);
    CHECK(centeredX.cols() == 10);
    CHECK(theoretical.rows() == 10);
    CHECK(theoretical.cols() == 10);
    CHECK(theoretical(0, 0) == doctest::Approx(291.0));
    CHECK(theoretical(4, 4) == doctest::Approx(301.0));
    CHECK(theoretical(8, 8) == doctest::Approx(284.7875));
    CHECK(theoretical(0, 4) == doctest::Approx(0.0));
    CHECK(theoretical(0, 8) == doctest::Approx(-87.0));
    CHECK(theoretical(4, 8) == doctest::Approx(277.5));

    const auto n = centeredX.cols();
    {
        auto gram = Sparsepc::EigenSolver<Scalar>{}.maximumValueElement(centeredX);
        auto eigen =
            Sparsepc::EigenLibEigenSolver<Scalar>{}.maximumValueElement(centeredX);

        Index idxMaxCoeff = 0;
        gram.vector.cwiseAbs().maxCoeff(&idxMaxCoeff);
        gram.vector *= sign(gram.vector[idxMaxCoeff]);
        eigen.vector.cwiseAbs().maxCoeff(&idxMaxCoeff);
        eigen.vector *= sign(eigen.vector[idxMaxCoeff]);

        CHECK(std::abs(gram.value - eigen.value) <= 1e-4);
        {
            const auto norm = (gram.vector - eigen.vector).norm();
            const auto ok = norm < 1e-5 && norm > 1e-6;
            CHECK(ok);
        }
    }

    const Index k0 = 4;
    const Index k1 = 4;

    {
        using BackwardGspca = Sparsepc::linearmodel::BackwardGspca<Scalar>;
        const BackwardGspca::Param param{{k0, k1}};
        const auto sparseEigenElements = BackwardGspca{param}.run(centeredX);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.501963, 0.500532, 0.496671, 0.500818, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.496833, 0.502276, 0.501426, 0.499448, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
    }

    {
        using ForwardGspca = Sparsepc::linearmodel::ForwardGspca<Scalar>;
        const ForwardGspca::Param param{{k0, k1}};
        const auto sparseEigenElements = ForwardGspca{param}.run(centeredX);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(178569.2040900).epsilon(1e-6));
            Vector v(n);
            v << 0, -0.4199159, 0, -0.4174335, 0, 0, 0, 0, 0.5718904, 0.5677687;
            CHECK((sparseEigenElements[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
    }

    {
        using ParallelGspca = Sparsepc::linearmodel::ParallelGspca<Scalar>;
        const ParallelGspca::Param param{{k0, k1}};
        const auto sparseEigenElements = ParallelGspca{param}.run(centeredX);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
    }

    {
        using Dca = Sparsepc::linearmodel::Dca<Scalar>;
        const Dca::Param param{{k0, k1}};
        const auto sparseEigenElements = Dca{param}.run(centeredX);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(284009.2987747).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5026805, 0.5016918, 0, 0, 0.4980111, 0.4975969;
            CHECK((sparseEigenElements[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
    }

    {
        using ContiguousFacetsFinder =  Sparsepc::linearmodel::ContiguousFacetsFinder<Scalar>;
        const ContiguousFacetsFinder::Param param{{k0, k1}};
        const auto sparseEigenElements =
            ContiguousFacetsFinder{param}.run(centeredX);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
    }

    {
        using MavIterativeElimination = Sparsepc::linearmodel::MavIterativeElimination<Scalar>;
        const MavIterativeElimination::Param param{{k0, k1}};
        const auto sparseEigenElements =
            MavIterativeElimination{param}.run(centeredX);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
    }

    {
        using AmvlIterativeElimination = Sparsepc::linearmodel::AmvlIterativeElimination<Scalar>;
        const AmvlIterativeElimination::Param param{{k0, k1}};
        const auto sparseEigenElements =
            AmvlIterativeElimination{param}.run(centeredX);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((sparseEigenElements[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((sparseEigenElements[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
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
                centeredX, BackwardGspca::ImplementationParam{},
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
            CHECK(validatedComponents[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.501963, 0.500532, 0.496671, 0.500818, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.496833, 0.502276, 0.501426, 0.499448, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
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
                centeredX, ForwardGspca::ImplementationParam{},
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
            CHECK(validatedComponents[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(178569.2040900).epsilon(1e-6));
            Vector v(n);
            v << 0, -0.4199159, 0, -0.4174335, 0, 0, 0, 0, 0.5718904, 0.5677687;
            CHECK((validatedComponents[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
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
                centeredX, Dca::ImplementationParam{}, validatedComponents,
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
            CHECK(validatedComponents[0].value ==
                  doctest::Approx(284009.2987747).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5026805, 0.5016918, 0, 0, 0.4980111, 0.4975969;
            CHECK((validatedComponents[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
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
                    centeredX, ContiguousFacetsFinder::ImplementationParam{},
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
            CHECK(validatedComponents[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
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
                    centeredX, MavIterativeElimination::ImplementationParam{},
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
            CHECK(validatedComponents[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
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
                    centeredX, AmvlIterativeElimination::ImplementationParam{},
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
            CHECK(validatedComponents[0].value ==
                  doctest::Approx(290445.4200583).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0.5019632, 0.5005321, 0.4966709, 0.5008179, 0, 0;
            CHECK((validatedComponents[0].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(226591.2637618).epsilon(1e-6));
            Vector v(n);
            v << 0.4968329, 0.5022760, 0.5014259, 0.4994476, 0, 0, 0, 0, 0, 0;
            CHECK((validatedComponents[1].vector - v).norm() ==
                  doctest::Approx(0.0).epsilon(1e-6));
        }
    }
}

TEST_CASE("Sparsepc version")
{
    static_assert(Sparsepc::metadata::libVersion == std::string_view("0.2.0"));
    CHECK(std::string(Sparsepc::metadata::libVersion) == std::string("0.2.0"));
}
