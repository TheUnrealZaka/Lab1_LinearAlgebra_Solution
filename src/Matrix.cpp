#include "Matrix.hpp"
#include <stdexcept>

Matrix Matrix::Identity(std::size_t n) 
{
    Matrix I(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        I.At(i, i) = 1;
    }
    return I;
}

double& Matrix::At(std::size_t i, std::size_t j) 
{
    if (i >= rows || j >= cols) throw std::out_of_range("Matrix::At (non-const): índex fora de rang");
    return a[i * cols + j];
}
double Matrix::At(std::size_t i, std::size_t j) const 
{
    if (i >= rows || j >= cols) throw std::out_of_range("Matrix::At (const): índex fora de rang");
    return a[i * cols + j];
}

Vec Matrix::Multiply(const Vec& x, OpsCounter* op) const 
{
    if (cols != x.size()) {
        throw std::invalid_argument("Matrix::Multiply(mat-vec): dimensions incompatibles");
    }

    Vec y(rows);
    if (rows == 0 || cols == 0) {
        return y;
    }

    for (std::size_t i = 0; i < rows; ++i) {
        const std::size_t row_offset = i * cols;
        // Primer terme (si existeix) per evitar una addició innecessària
        double acc = a[row_offset] * x[0];
        if (op) op->IncMul();

        for (std::size_t j = 1; j < cols; ++j) {
            acc += a[row_offset + j] * x[j];
            if (op) {
                op->IncMul();
                op->IncAdd();
            }
        }
        y[i] = acc;
    }

    return y;
}

Matrix Matrix::Multiply(const Matrix& B, OpsCounter* op) const 
{
    if (cols != B.rows) {
        throw std::invalid_argument("Matrix::Multiply(mat-mat): dimensions incompatibles");
    }

    Matrix C(rows, B.cols);
    if (rows == 0 || cols == 0 || B.cols == 0) {
        return C;
    }

    // Simple triple bucle, amb comptatge d'operacions
    for (std::size_t i = 0; i < rows; ++i) {
        const std::size_t a_row_offset = i * cols;
        for (std::size_t k = 0; k < B.cols; ++k) {
            // Primer producte fora del bucle per evitar suma innecessària
            double acc = a[a_row_offset + 0] * B.At(0, k);
            if (op) op->IncMul();

            for (std::size_t j = 1; j < cols; ++j) {
                acc += a[a_row_offset + j] * B.At(j, k);
                if (op) {
                    op->IncMul();
                    op->IncAdd();
                }
            }
            C.At(i, k) = acc;
        }
    }

    return C;
}
