#include <doctest/doctest.h>
#include <source/simu.hpp>
#include <sparsepc/core.hpp>
#include <sparsepc/version.hpp>

#include <Eigen/Dense>

#include <cstdlib>
#include <string>

TEST_CASE("Sparsepc Gram")
{
    using Scalar = double;
    const auto sigma = sparsepc::linearmodel::pitprops<Scalar>();

    const auto eeGram =
        sparsepc::EigenSolver<Scalar>{}.maximumValueElement(sigma);
    const auto eeEigen =
        sparsepc::EigenLibEigenSolver<Scalar>{}.maximumValueElement(sigma);

    CHECK(std::abs(eeGram.value - eeEigen.value) <= 1e-4);
    CHECK((eeGram.vector - eeEigen.vector).norm() <= 1e-2);
}

TEST_CASE("Sparsepc version")
{
    static_assert(std::string_view(SPARSEPC_VERSION) ==
                  std::string_view("0.1.0"));
    CHECK(std::string(SPARSEPC_VERSION) == std::string("0.1.0"));
}
