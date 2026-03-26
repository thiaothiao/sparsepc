#ifndef SPARSEPC_UTILS_HPP
#define SPARSEPC_UTILS_HPP

#include <concepts>

#include <Eigen/Dense>

namespace sparsepc
{
	using Index = Eigen::Index;

	using Vectori = Eigen::Matrix<Index, Eigen::Dynamic, 1>;

	template<std::floating_point ScalarType>
	using Vector = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>;

	template<std::floating_point ScalarType>
	using Matrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;

	template<std::floating_point ScalarType>
	using RMMatrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
}

#endif //SPARSEPC_UTILS_HPP
