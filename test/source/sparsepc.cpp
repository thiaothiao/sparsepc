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
    const auto sim =
        Sparsepc::linearmodel::simulation<Scalar>(numberOfSamples, seed);
    const auto featureMatrix = Sparsepc::standardScale(sim.observed);
    const auto theoretical = sim.theoreticalCovariance();

    CHECK(featureMatrix.rows() == numberOfSamples);
    CHECK(featureMatrix.cols() == 10);
    CHECK(theoretical.rows() == 10);
    CHECK(theoretical.cols() == 10);
    CHECK(theoretical(0, 0) == doctest::Approx(291.0));
    CHECK(theoretical(4, 4) == doctest::Approx(301.0));
    CHECK(theoretical(8, 8) == doctest::Approx(284.7875));
    CHECK(theoretical(0, 4) == doctest::Approx(0.0));
    CHECK(theoretical(0, 8) == doctest::Approx(-87.0));
    CHECK(theoretical(4, 8) == doctest::Approx(277.5));

    const auto n = featureMatrix.cols();
    {
        auto eigen =
            Sparsepc::EigenLibEigenSolver<Scalar>{}.maximumValueElement(
                featureMatrix);
        auto spectra =
            Sparsepc::SpectraLibEigenSolver<Scalar>{}.maximumValueElement(
                featureMatrix);

        Index idxMaxCoeff = 0;
        eigen.vector.cwiseAbs().maxCoeff(&idxMaxCoeff);
        eigen.vector *= sign(eigen.vector[idxMaxCoeff]);
        spectra.vector.cwiseAbs().maxCoeff(&idxMaxCoeff);
        spectra.vector *= sign(spectra.vector[idxMaxCoeff]);

        CHECK(std::abs(eigen.value - spectra.value) <= 1e-8);
        {
            const auto norm = (eigen.vector - spectra.vector).norm();
            const auto ok = norm < 1e-10;
            CHECK(ok);
        }
    }

    const Index k0 = 4;
    const Index k1 = 4;

    {
        using BackwardGspca = Sparsepc::linearmodel::BackwardGspca<Scalar>;
        const BackwardGspca::Param param{{k0, k1}};
        const auto sparseEigenElements =
            BackwardGspca{param}.run(featureMatrix);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (sparseEigenElements[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (sparseEigenElements[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
    }

    {
        using ForwardGspca = Sparsepc::linearmodel::ForwardGspca<Scalar>;
        const ForwardGspca::Param param{{k0, k1}};
        const auto sparseEigenElements = ForwardGspca{param}.run(featureMatrix);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (sparseEigenElements[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(545.4435852924).epsilon(1e-6));
            Vector v(n);
            v << 0.499929, 0.499869, 0, 0, 0, 0, 0, 0, -0.501845, -0.498350;
            const auto ok = (sparseEigenElements[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
    }

    {
        using ParallelGspca = Sparsepc::linearmodel::ParallelGspca<Scalar>;
        const ParallelGspca::Param param{{k0, k1}};
        const auto sparseEigenElements =
            ParallelGspca{param}.run(featureMatrix);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (sparseEigenElements[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (sparseEigenElements[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
    }

    {
        using Dca = Sparsepc::linearmodel::Dca<Scalar>;
        const Dca::Param param{{k0, k1}};
        const auto sparseEigenElements = Dca{param}.run(featureMatrix);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(779.5772672134).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0, 0.500078, 0.499909, 0, 0.499673, 0.500338;
            const auto ok = (sparseEigenElements[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(793.9013045323).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (sparseEigenElements[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
    }

    {
        using ContiguousFacetsFinder =  Sparsepc::linearmodel::ContiguousFacetsFinder<Scalar>;
        const ContiguousFacetsFinder::Param param{{k0, k1}};
        const auto sparseEigenElements =
            ContiguousFacetsFinder{param}.run(featureMatrix);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (sparseEigenElements[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (sparseEigenElements[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
    }

    {
        using MavIterativeElimination = Sparsepc::linearmodel::MavIterativeElimination<Scalar>;
        const MavIterativeElimination::Param param{{k0, k1}};
        const auto sparseEigenElements =
            MavIterativeElimination{param}.run(featureMatrix);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (sparseEigenElements[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (sparseEigenElements[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
    }

    {
        using AmvlIterativeElimination = Sparsepc::linearmodel::AmvlIterativeElimination<Scalar>;
        const AmvlIterativeElimination::Param param{{k0, k1}};
        const auto sparseEigenElements =
            AmvlIterativeElimination{param}.run(featureMatrix);
        {
            CHECK(sparseEigenElements[0].value ==
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (sparseEigenElements[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {

            CHECK(sparseEigenElements[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (sparseEigenElements[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (sparseEigenElements[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
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
                featureMatrix, BackwardGspca::ImplementationParam{},
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
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (validatedComponents[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (validatedComponents[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
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
                featureMatrix, ForwardGspca::ImplementationParam{},
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
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (validatedComponents[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(545.4435852924).epsilon(1e-6));
            Vector v(n);
            v << 0.499929, 0.499869, 0, 0, 0, 0, 0, 0, -0.501845, -0.498350;
            const auto ok = (validatedComponents[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
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
                featureMatrix, Dca::ImplementationParam{}, validatedComponents,
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
                  doctest::Approx(779.5772672134).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, 0, 0.500078, 0.499909, 0, 0.499673, 0.500338;
            const auto ok = (validatedComponents[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(793.9013045323).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (validatedComponents[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
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
                    featureMatrix,
                    ContiguousFacetsFinder::ImplementationParam{},
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
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (validatedComponents[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (validatedComponents[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
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
                    featureMatrix,
                    MavIterativeElimination::ImplementationParam{},
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
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (validatedComponents[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (validatedComponents[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
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
                    featureMatrix,
                    AmvlIterativeElimination::ImplementationParam{},
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
                  doctest::Approx(794.3905007324).epsilon(1e-6));
            Vector v(n);
            v << 0, 0, 0, 0, -0.500062, -0.499957, -0.500009, -0.499969, 0, 0;
            const auto ok = (validatedComponents[0].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[0].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
        {
            CHECK(validatedComponents[1].value ==
                  doctest::Approx(793.9013045324).epsilon(1e-6));
            Vector v(n);
            v << -0.500046, -0.500027, -0.499985, -0.499940, 0, 0, 0, 0, 0, 0;
            const auto ok = (validatedComponents[1].vector - v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4) ||
                            (validatedComponents[1].vector + v).norm() ==
                                doctest::Approx(0.0).epsilon(1e-4);
            CHECK(ok);
        }
    }
}

TEST_CASE("Sparsepc version")
{
    static_assert(Sparsepc::metadata::libVersion == std::string_view("0.2.0"));
    CHECK(std::string(Sparsepc::metadata::libVersion) == std::string("0.2.0"));
}
