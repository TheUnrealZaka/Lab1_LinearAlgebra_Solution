#include "Matrix.hpp"
#include "DatasetIO.hpp"
#include <random>
#include <cmath>
#include <string>
#include <cassert>
#include <iostream>
#include <filesystem>

using std::size_t;

static Matrix rand_mat(int r, int c, std::mt19937& rng) {
    std::uniform_real_distribution<double> U(-1.0, 1.0);
    Matrix M(r, c, 0.0);
    for (int i = 0; i < r; ++i) for (int j = 0; j < c; ++j) M.At(i, j) = U(rng);
    return M;
}

int main() {
    std::filesystem::create_directories("datasets");

    for (int n : {500, 600, 700, 800}) {
        std::mt19937 rng(1234u + n);
        std::uniform_real_distribution<double> U(-1.0, 1.0);

        // ---------- CASOS "OK" ----------
        Matrix A(n, n, 0.0), B(n, n, 0.0);
        Vec x(n), rhs;
        for (int i = 0; i < n; ++i) {
            double rowsum = 0.0;
            for (int j = 0; j < n; ++j) {
                if (i != j) { double v = U(rng); A.At(i, j) = v; rowsum += std::fabs(v); }
                B.At(i, j) = U(rng);
            }
            A.At(i, i) = rowsum + 1.0;     // A ben condicionada
            x[(size_t)i] = U(rng);
        }
        Vec y = A.Multiply(x, nullptr);       // oracle MatVec
        Matrix C = A.Multiply(B, nullptr);    // oracle MatMul
        rhs = y;                               // rhs per Solve

        SaveMatrixBin("datasets/A_" + std::to_string(n) + ".bin", A);
        SaveMatrixBin("datasets/B_" + std::to_string(n) + ".bin", B);
        SaveVectorBin("datasets/x_" + std::to_string(n) + ".bin", x);
        SaveVectorBin("datasets/rhs_" + std::to_string(n) + ".bin", rhs);
        SaveVectorBin("datasets/y_" + std::to_string(n) + ".bin", y);
        SaveMatrixBin("datasets/C_" + std::to_string(n) + ".bin", C);

        // ---------- CASOS "DIM MISMATCH" ----------
        // MatVec: x_bad de mida n-1
        Vec x_bad((size_t)std::max(1, n - 1));
        for (int i = 0; i < (int)x_bad.size(); ++i) x_bad[(size_t)i] = U(rng);
        SaveVectorBin("datasets/x_bad_" + std::to_string(n) + ".bin", x_bad);

        // MatMul: B_bad de mida n x (n-1) (sense capçalera -> fitxer amb (n*(n-1)) doubles)
        Matrix B_bad = rand_mat(n, std::max(1, n - 1), rng);
        SaveMatrixBin("datasets/B_bad_" + std::to_string(n) + ".bin", B_bad);

        // ---------- CASOS "ZERO PIVOT (no singular) ----------
        // A_zeropiv: pivot (0,0)=0 però fila 1 té element a (1,0) no nul -> parcial pivot ha de resoldre
        Matrix A_zeropiv = A;
        A_zeropiv.At(0, 0) = 0.0;
        if (std::fabs(A_zeropiv.At(1, 0)) < 0.2) A_zeropiv.At(1, 0) = (A_zeropiv.At(1, 0) >= 0 ? +0.5 : -0.5);
        Vec x_true = x; // qualsevol
        Vec rhs_zeropiv = A_zeropiv.Multiply(x_true, nullptr);

        SaveMatrixBin("datasets/A_zeropiv_" + std::to_string(n) + ".bin", A_zeropiv);
        SaveVectorBin("datasets/rhs_zeropiv_" + std::to_string(n) + ".bin", rhs_zeropiv);

        // ---------- CASOS "SINGULAR" ----------
        // A_sing: dupliquem la fila 0 a la fila 1 (rang deficient)
        Matrix A_sing = A;
        for (int j = 0; j < n; ++j) A_sing.At(1, j) = A_sing.At(0, j);
        Vec rhs_sing = A_sing.Multiply(x_true, nullptr); // coherent

        SaveMatrixBin("datasets/A_sing_" + std::to_string(n) + ".bin", A_sing);
        SaveVectorBin("datasets/rhs_sing_" + std::to_string(n) + ".bin", rhs_sing);

        // informes ràpids
        auto szB = std::filesystem::file_size("datasets/B_" + std::to_string(n) + ".bin");
        auto szBb = std::filesystem::file_size("datasets/B_bad_" + std::to_string(n) + ".bin");
        std::cout << "OK n=" << n
            << "  A:" << A.a.size() << "  B:" << B.a.size()
            << "  y:" << y.size() << "  C:" << C.a.size()
            << "  x_bad:" << x_bad.size()
            << "\nB_" << n << ".bin bytes=" << szB << "  B_bad_" << n << ".bin bytes=" << szBb << "\n";
    }
    return 0;
}