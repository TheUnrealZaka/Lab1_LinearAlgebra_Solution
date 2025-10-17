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

// Funció auxiliar per calcular el cost de multiplicar matrius de dimensions (m x n) i (n x p)
static std::size_t MulCost(std::size_t m, std::size_t n, std::size_t p)
{
    return m * n * p;
}

// Multiplicació de tres matrius amb optimització de l'ordre d'operacions
Matrix Multiply3(const Matrix& A, const Matrix& B, const Matrix& C, OpsCounter* op)
{
    // Validació de dimensions
    if (A.cols != B.rows) {
        throw std::invalid_argument("A*B*C: dimensions incompatibles");
    }
    if (B.cols != C.rows) {
        throw std::invalid_argument("A*B*C: dimensions incompatibles");
    }
    
    std::size_t m = A.rows; // nombre de files de A
    std::size_t n = A.cols; // nombre de columnes de A i files de B
    std::size_t p = B.cols; // nombre de columnes de B i files de C
    std::size_t q = C.cols; // nombre de columnes de C

    std::size_t cost_ab_c = MulCost(m, n, p) + MulCost(m, p, q); // Cost d'(A*B)*C
    
    std::size_t cost_a_bc = MulCost(n, p, q) + MulCost(m, n, q); // Cost d'A*(B*C)

    // Triem l'ordre amb menor cost computacional
    if (cost_ab_c <= cost_a_bc) {
        Matrix AB = A.Multiply(B, op);
        return AB.Multiply(C, op);
    } else {
        Matrix BC = B.Multiply(C, op);
        return A.Multiply(BC, op);
    }
}
