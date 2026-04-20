#pragma once

#include <concepts>
#include <fstream>
#include <iostream>
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
    void saveData(std::string fileName, const RMMatrix<ScalarType> &matrix,
                  const char sep = ',')
    { // TODO optimize
        // https://eigen.tuxfamily.org/dox/structEigen_1_1IOFormat.html
        const static Eigen::IOFormat CSVFormat(Eigen::FullPrecision,
                                               Eigen::DontAlignCols,
                                               std::string(1, sep), "\n");

        std::ofstream file(fileName);
        if (file.is_open())
        {
            file << matrix.format(CSVFormat);
            file.close();
        }
    }

    template <std::floating_point ScalarType>
    Matrix<ScalarType> openData(std::string fileToOpen, const char sep = ',')
    { // TODO optimize memory usage

        using Scalar = ScalarType;
        // the inspiration for creating this function was drawn from here (I did
        // NOT copy and paste the code)
        // https://stackoverflow.com/questions/34247057/how-to-read-csv-file-and-assign-to-eigen-matrix

        // the input is the file: "fileToOpen.csv":
        // a,b,c
        // d,e,f
        // This function converts input file data into the Eigen matrix format
        // the matrix entries are stored in this variable row-wise. For example
        // if we have the matrix: M=[a b c
        //	  d e f]
        // the entries are stored as matrixEntries=[a,b,c,d,e,f], that is the
        // variable "matrixEntries" is a row vector later on, this vector is
        // mapped into the Eigen matrix format
        std::vector<Scalar> matrixEntries;
        // in this object we store the data from the matrix
        std::ifstream matrixDataFile(fileToOpen);
        // this variable is used to store the row of the matrix that contains
        // commas
        std::string matrixRowString;
        // this variable is used to store the matrix entry;
        std::string matrixEntry;
        // this variable is used to track the number of rows
        int matrixRowNumber = 0;
        while (std::getline(
            matrixDataFile,
            matrixRowString)) // here we read a row by row of matrixDataFile and
                              // store every line into the string variable
                              // matrixRowString
        {
            std::stringstream matrixRowStringStream(
                matrixRowString); // convert matrixRowString that is a string to
                                  // a stream variable.

            while (std::getline(
                matrixRowStringStream, matrixEntry,
                sep)) // here we read pieces of the stream matrixRowStringStream
                      // until every comma, and store the resulting character
                      // into the matrixEntry
            {
                matrixEntries.push_back(static_cast<Scalar>(std::stod(
                    matrixEntry))); // here we convert the string to double and
                                    // fill in the row vector storing all the
                                    // matrix entries
            }
            matrixRowNumber++; // update the column numbers
        }
        // here we convet the vector variable into the matrix and return the
        // resulting object, note that matrixEntries.data() is the pointer to
        // the first memory location at which the entries of the vector
        // matrixEntries are stored;
        const RMMatrix<ScalarType> matrix = Eigen::Map<RMMatrix<Scalar>>(
            matrixEntries.data(), matrixRowNumber,
            matrixEntries.size() / matrixRowNumber);
        return matrix;
    }
} // namespace sparsepc
