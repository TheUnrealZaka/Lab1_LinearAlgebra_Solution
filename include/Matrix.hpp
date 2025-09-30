#pragma once
#include <vector>
#include <cstddef>
#include "OpsCounter.hpp"

using Vec = std::vector<double>;

struct Matrix 
{
    std::size_t rows = 0, cols = 0;
    std::vector<double> a; // row-major

    Matrix() = default;
    Matrix(std::size_t r, std::size_t c, double val = 0.0) : rows(r), cols(c), a(r* c, val) 
    {
    }

    static Matrix Identity(std::size_t n);               // TODO (Ex1)
    double& At(std::size_t i, std::size_t j);            // TODO (Ex1)
    double  At(std::size_t i, std::size_t j) const;      // TODO (Ex1)

    Vec    Multiply(const Vec& x, OpsCounter* op = nullptr) const;       // TODO (Ex1)
    Matrix Multiply(const Matrix& B, OpsCounter* op = nullptr) const;    // TODO (Ex1)

    Vec operator*(const Vec& x) const 
    { 
        return Multiply(x, nullptr); 
    }
    Matrix operator*(const Matrix& B) const 
    { 
        return Multiply(B, nullptr); 
    }

    void SwapRows(std::size_t i, std::size_t j) 
    {
        if (i == j) return;
        for (std::size_t c = 0; c < cols; ++c) std::swap(At(i, c), At(j, c));
    }
    bool IsSquare() const 
    { 
        return rows == cols; 
    }
};