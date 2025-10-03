#include "Matrix.hpp"
#include <stdexcept>

Matrix Matrix::Identity(std::size_t n) 
{
    // Creem una matriu quadrada n×n inicialitzada a zero.
    Matrix I(n, n);

    // Recorrem la diagonal principal i hi col·loquem uns.
    for (std::size_t i = 0; i < n; ++i) {
        I.At(i, i) = 1;
    }
    return I;
}

double& Matrix::At(std::size_t i, std::size_t j) 
{
    // En disposició row-major, l'índex pla és i*cols + j.
    return a[i * cols + j];
}
double Matrix::At(std::size_t i, std::size_t j) const 
{
    // En disposició row-major, l'índex pla és i*cols + j.
    return a[i * cols + j];
}

Vec Matrix::Multiply(const Vec& x, OpsCounter* op) const 
{
    if (cols != x.size()) {
        throw std::invalid_argument("Matrix::Multiply(mat-vec): dimensions incompatibles");
    }

    // Resultat inicialitzat a zero amb una entrada per fila.
    Vec y(rows, 0.0);
    if (rows == 0 || cols == 0) {
        return y;
    }

    // Per cada fila calculem el producte escalar amb el vector.
    for (std::size_t i = 0; i < rows; ++i) {
        const std::size_t row_offset = i * cols;

        // Primer terme (fora del bucle intern) per evitar una suma innecessària.
        double acc = a[row_offset] * x[0];
        if (op) {
            op->IncMul();
        }

        // Afegim els termes restants acumulant multiplicacions i sumes.
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

    // Matriu resultat inicialitzada a zero de mida (files A) × (columnes B).
    Matrix C(rows, B.cols, 0.0);
    if (rows == 0 || cols == 0 || B.cols == 0) {
        return C;
    }

    // Recorrem cada combinació fila A / columna B.
    for (std::size_t i = 0; i < rows; ++i) {
        const std::size_t a_row_offset = i * cols;
        for (std::size_t k = 0; k < B.cols; ++k) {
            // Primer producte fora del bucle per reduir sumes.
            double acc = a[a_row_offset] * B.At(0, k);
            if (op) {
                op->IncMul();
            }

            // Resta d'entrades: acumulació de multiplicacions i sumes.
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
