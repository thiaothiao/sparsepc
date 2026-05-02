#pragma once

#include <concepts>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <Eigen/Dense>

namespace sparsepc
{
    using Index = Eigen::Index;
    using Vectori = Eigen::Matrix<Index, Eigen::Dynamic, 1>;
    template <std::floating_point ScalarType>
    using Vector = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>;
    template <std::floating_point ScalarType>
    using Matrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic,
                                 Eigen::ColMajor>;
    template <std::floating_point ScalarType>
    using RMMatrix = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic,
                                   Eigen::RowMajor>;

    template <std::floating_point ScalarType>
    Matrix<ScalarType> openData(std::string fileToOpen, const char sep = ',')
    try
    {
        std::ifstream matrixDataFile(fileToOpen);
        if (matrixDataFile.is_open() && !matrixDataFile.eof())
        { // TODO optimize memory usage
            using Scalar = ScalarType;
            std::vector<Scalar> matrixEntries;
            std::string matrixRowString;
            std::string matrixEntry;
            int matrixRowNumber = 0;
            int matrixColumnNumber = 0;
            bool columnNumberInitialized = false;

            while (std::getline(matrixDataFile, matrixRowString))
            {
                std::stringstream matrixRowStringStream(matrixRowString);
                int currentColumnNumber = 0;
                while (std::getline(matrixRowStringStream, matrixEntry, sep))
                {
                    matrixEntries.push_back(
                        static_cast<Scalar>(std::stod(matrixEntry)));
                    ++currentColumnNumber;
                }

                if (columnNumberInitialized)
                {
                    if (matrixColumnNumber != currentColumnNumber)
                    {
                        std::cout << "Fatal error: Non constant column number."
                                  << std::endl;
                        return {};
                    }
                }
                else
                {
                    matrixColumnNumber = currentColumnNumber;
                    columnNumberInitialized = true;
                }

                if (matrixRowStringStream.bad())
                {
                    std::cout << "Fatal error: Stream corrupted or "
                                 "hardware failure."
                              << std::endl;
                    return {};
                }

                ++matrixRowNumber;
            }

            if (matrixDataFile.bad())
            {
                std::cout << "Fatal error: file data corrupted or "
                             "hardware failure."
                          << std::endl;
                return {};
            }

            if (matrixRowNumber > 0 && matrixColumnNumber > 0 &&
                !matrixEntries.empty())
            {
                const RMMatrix<ScalarType> matrix =
                    Eigen::Map<RMMatrix<Scalar>>(matrixEntries.data(),
                                                 matrixRowNumber,
                                                 matrixColumnNumber);
                return matrix;
            }
        }

        return {};
    }
    catch (...)
    {
        std::cout << "An exception pops up!!!" << std::endl;
        return {};
    }
} // namespace sparsepc
