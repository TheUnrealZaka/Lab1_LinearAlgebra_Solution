// main_eval.cpp
#include <iostream>
#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include "Matrix.hpp"
#include "Solve.hpp"
#include "LinAlg.hpp"
#include "DatasetIO.hpp"
#include "Timer.hpp"
#include "OpsCounter.hpp"

// ANSI colors
static constexpr const char* GREEN = "\x1b[32m";
static constexpr const char* RED = "\x1b[31m";
static constexpr const char* YELL = "\x1b[33m";
static constexpr const char* CYAN = "\x1b[36m";
static constexpr const char* RESET = "\x1b[0m";

// Forwards (no cal .hpp nou)
Matrix Multiply3(const Matrix& A, const Matrix& B, const Matrix& C, OpsCounter* op = nullptr);
namespace LinAlg { SolveReport SolvePivotAugmented(Matrix Ab, double tol); }

// Utils
static Matrix RandMat(std::size_t r, std::size_t c, std::mt19937& g) {
    std::uniform_real_distribution<double> U(-1.0, 1.0);
    Matrix M(r, c, 0.0);
    for (std::size_t i = 0; i < r; ++i) for (std::size_t j = 0; j < c; ++j) M.At(i, j) = U(g);
    return M;
}
static Vec RandVec(std::size_t n, std::mt19937& g) {
    std::uniform_real_distribution<double> U(-1.0, 1.0);
    Vec v(n); for (auto& t : v) t = U(g); return v;
}

static std::size_t cost(std::size_t m, std::size_t n, std::size_t p) { return m * n * p; }

static double RelErr(const Vec& a, const Vec& b) {
    if (a.size() != b.size()) return std::numeric_limits<double>::infinity();
    Vec r = a; for (std::size_t i = 0; i < r.size(); ++i) r[i] -= b[i];
    double num = LinAlg::L2Norm(r, nullptr), den = LinAlg::L2Norm(b, nullptr);
    return den == 0.0 ? num : (num / den);
}

int main() {
    std::mt19937 rng(1234);

    // ---------- Bloc 1: MatMul3 ----------
    std::size_t m = 200, n = 300, p = 100, q = 400;
    Matrix A = RandMat(m, n, rng);
    Matrix B = RandMat(n, p, rng);
    Matrix C = RandMat(p, q, rng);

    // Expectatives teòriques
    std::size_t mul_ab_c = cost(m, n, p) + cost(m, p, q);
    std::size_t mul_a_bc = cost(n, p, q) + cost(m, n, q);
    std::cout << CYAN << "Matrix sizes: A(" << m << "x" << n << ") B(" << n << "x" << p << ") C(" << p << "x" << q << ")" << RESET << "\n";
    std::cout << "[MatMul3] mul((AB)C)=" << mul_ab_c << "  mul(A(BC))=" << mul_a_bc
        << "  -> teoricament millor: " << (mul_ab_c <= mul_a_bc ? "(AB)C" : "A(BC)") << "\n";

    // Execució amb comptador: Multiply3 escull l’ordre òptim
    OpsCounter op3; Timer t3; t3.Tic();
    Matrix R = Multiply3(A, B, C, &op3);
    double t_ms3 = t3.TocMs();
    std::cout << "[MatMul3] mul=" << op3.mul << " add=" << op3.add << " time_ms=" << t_ms3 << "  shape=" << R.rows << "x" << R.cols << "\n";

    auto mul_teo = std::min(mul_ab_c, mul_a_bc);
    double rel = std::fabs(double(op3.mul) - double(mul_teo)) / double(mul_teo);
    std::cout << "[MatMul3] mul check: "
        << (rel <= 0.02 ? GREEN : RED) << (rel <= 0.02 ? "OK" : "KO") << RESET
        << " (rel=" << rel << ")\n";

    // (1) CAS D'ERROR INTENCIONAT: dimensions incompatibles a MatMul3
    try {
        Matrix Bbad(n + 1, p, 0.0);     // Bbad.rows != A.cols -> ha de fer throw
        (void)Multiply3(A, Bbad, C, nullptr);
        std::cout << "[MatMul3/Incompatible] " << RED << "KO (no ha llençat excepcio)" << RESET << "\n";
    }
    catch (const std::exception&) {
        std::cout << "[MatMul3/Incompatible] " << GREEN << "OK (throw detectat)" << RESET << "\n";
    }

    // ---------- Bloc 2: Solve Pivot (clàssic) vs Augmented ----------
    for (int nsize : {500, 600}) {
        Matrix As(nsize, nsize, 0.0);
        Vec b(nsize, 0.0);
        // A diagonal dominant i b random (igual que generate_dataset)
        std::uniform_real_distribution<double> U(-1.0, 1.0);
        for (int i = 0; i < nsize; ++i) {
            double rowsum = 0.0;
            for (int j = 0; j < nsize; ++j) if (i != j) { double v = U(rng); As.At(i, j) = v; rowsum += std::fabs(v); }
            As.At(i, i) = rowsum + 1.0;
            b[(size_t)i] = U(rng);
        }

        Matrix Ab(nsize, nsize + 1, 0.0);
        for (int i = 0; i < nsize; ++i) {
            for (int j = 0; j < nsize; ++j) Ab.At(i, j) = As.At(i, j);
            Ab.At(i, nsize) = b[(size_t)i];
        }

        auto r_ref = LinAlg::SolvePartialPivot(As, b, 1e-12);
        std::cout << "[Solve n=" << nsize << "] classic:   ms=" << r_ref.ms
            << " ops(mul=" << r_ref.ops.mul << ",add=" << r_ref.ops.add
            << ",div=" << r_ref.ops.div_ << ",cmp=" << r_ref.ops.cmp << ",swp=" << r_ref.ops.swp
            << ")  rel=" << r_ref.rel_resid << "\n";

        auto r_aug = LinAlg::SolvePivotAugmented(Ab, 1e-12);
        std::cout << "[Solve n=" << nsize << "] augmented: ms=" << r_aug.ms << "  ops(mul=" << r_aug.ops.mul
            << ",add=" << r_aug.ops.add << ",div=" << r_aug.ops.div_ << ",cmp=" << r_aug.ops.cmp << ",swp=" << r_aug.ops.swp
            << ")  rel=" << r_aug.rel_resid << "\n";
    }

    // (2) CAS INCOMPATIBLE SolvePivotAugmented: Ab no és n x (n+1)
    try {
        Matrix badAb(5, 5, 0.0); // cols != rows+1
        (void)LinAlg::SolvePivotAugmented(badAb, 1e-12);
        std::cout << "[Solve/Ab incompatible] " << RED << "KO (no ha llençat excepcio)" << RESET << "\n";
    }
    catch (const std::exception&) {
        std::cout << "[Solve/Ab incompatible] " << GREEN << "OK (throw detectat)" << RESET << "\n";
    }

    // (3) CAS AMB SWAPS: força pivotatge (swp>0) i comprova solucio
    {
        int n2 = 6;
        Matrix Aperm = Matrix::Identity(n2);
        // fem que el primer pivot sigui 0 i el segon 1: caldrà swap a k=0
        Aperm.SwapRows(0, 1);
        Vec x_true = RandVec(n2, rng);
        // b = Aperm * x_true
        Vec bperm(n2, 0.0);
        for (int i = 0; i < n2; ++i) {
            double acc = 0.0; for (int j = 0; j < n2; ++j) { acc += Aperm.At(i, j) * x_true[(size_t)j]; }
            bperm[(size_t)i] = acc;
        }

        // Classic
        auto r1 = LinAlg::SolvePartialPivot(Aperm, bperm, 1e-12);
        double xerr1 = RelErr(r1.x, x_true);
        std::cout << "[Solve/Swaps classic] swp=" << r1.ops.swp
            << " x-relerr=" << xerr1 << " -> "
            << (r1.ops.swp > 0 && xerr1 <= 1e-12 ? GREEN : RED)
            << (r1.ops.swp > 0 && xerr1 <= 1e-12 ? "OK" : "KO") << RESET << "\n";

        // Augmented
        Matrix Abp(n2, n2 + 1, 0.0);
        for (int i = 0; i < n2; ++i) { for (int j = 0; j < n2; ++j) Abp.At(i, j) = Aperm.At(i, j); Abp.At(i, n2) = bperm[(size_t)i]; }
        auto r2 = LinAlg::SolvePivotAugmented(Abp, 1e-12);
        double xerr2 = RelErr(r2.x, x_true);
        std::cout << "[Solve/Swaps augmented] swp=" << r2.ops.swp
            << " x-relerr=" << xerr2 << " -> "
            << (r2.ops.swp > 0 && xerr2 <= 1e-12 ? GREEN : RED)
            << (r2.ops.swp > 0 && xerr2 <= 1e-12 ? "OK" : "KO") << RESET << "\n";
    }

    // (4) CAS SINGULAR: una fila de zeros -> singular esperat
    {
        int n3 = 6;
        Matrix Asing(n3, n3, 0.0);
        for (int i = 0; i < n3 - 1; ++i) Asing.At(i, i) = 1.0; // última fila: tot zeros
        Vec bsing(n3, 0.0);

        // Classic
        auto rc = LinAlg::SolvePartialPivot(Asing, bsing, 1e-12);
        std::cout << "[Solve/Singular classic] "
            << (rc.singular ? GREEN : RED)
            << (rc.singular ? "OK (singular detectat)" : "KO (no detecta singular)") << RESET << "\n";

        // Augmented
        Matrix Absing(n3, n3 + 1, 0.0);
        for (int i = 0; i < n3; ++i) { for (int j = 0; j < n3; ++j) Absing.At(i, j) = Asing.At(i, j); Absing.At(i, n3) = bsing[(size_t)i]; }
        auto ra = LinAlg::SolvePivotAugmented(Absing, 1e-12);
        std::cout << "[Solve/Singular augmented] "
            << (ra.singular ? GREEN : RED)
            << (ra.singular ? "OK (singular detectat)" : "KO (no detecta singular)") << RESET << "\n";
    }

    std::cout << CYAN << ">>> Fi del main_eval (tots els checks executats)" << RESET << "\n";
    return 0;
}
