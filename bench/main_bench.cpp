#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "Matrix.hpp"
#include "LinAlg.hpp"
#include "Solve.hpp"
#include "DatasetIO.hpp"
#include "BenchConfig.hpp"
#include "Timer.hpp"
#include "OpsCounter.hpp"

static constexpr const char* G = "\x1b[32m", * R = "\x1b[31m", * Z = "\x1b[0m";

static bool approx_eq(std::size_t got, std::size_t exp, double tol) {
    double rel = std::fabs(double(got) - double(exp)) / double(exp ? exp : 1);
    return rel <= tol;
}
static double rel_err_vec(const Vec& y, const Vec& yref) {
    if (y.size() != yref.size()) return INFINITY;
    Vec r = y; for (size_t i = 0; i < r.size(); ++i) r[i] -= yref[i];
    double num = LinAlg::L2Norm(r, nullptr), den = LinAlg::L2Norm(yref, nullptr);
    return den == 0.0 ? num : num / den;
}
static double rel_err_mat(const Matrix& C, const Matrix& Cref) {
    if (C.rows != Cref.rows || C.cols != Cref.cols) return INFINITY;
    double num = 0, den = 0; for (size_t i = 0; i < C.a.size(); ++i) { double d = C.a[i] - Cref.a[i]; num += d * d; den += Cref.a[i] * Cref.a[i]; }
    return (den == 0.0) ? std::sqrt(num) : std::sqrt(num / std::max(den, 1e-300));
}

int main() {
    BenchConfig cfg;
    std::vector<int> ns = { 500,600,700,800 };
    bool all_ok = true;

    // ===== Ex1: MatVec/MatMul (OK) + casos DIM MISMATCH =====
    for (int n : ns) {
        Matrix A(n, n); LoadMatrixBin("datasets/A_" + std::to_string(n) + ".bin", A); A.rows = n; A.cols = n;
        Matrix B(n, n); LoadMatrixBin("datasets/B_" + std::to_string(n) + ".bin", B); B.rows = n; B.cols = n;
        Vec x; LoadVectorBin("datasets/x_" + std::to_string(n) + ".bin", x);

        // MatVec ok
        OpsCounter op1; Timer t1; t1.Tic(); Vec y = A.Multiply(x, &op1); double tms1 = t1.TocMs();
        std::size_t mul1 = std::size_t(n) * n, add1 = std::size_t(n) * (n - 1);
        bool ok_ops1 = approx_eq(op1.mul, mul1, cfg.matvec_ops_tol) && approx_eq(op1.add, add1, cfg.matvec_ops_tol);
        Vec yref; LoadVectorBin("datasets/y_" + std::to_string(n) + ".bin", yref);
        double ry = rel_err_vec(y, yref);
        bool ok_val1 = (ry <= 1e-12);
        bool ok1 = ok_ops1 && ok_val1;
        std::cout << "[Ex1][MatVec][n=" << n << "] " << (ok1 ? G : R) << (ok1 ? "PASS" : "FAIL") << Z
            << " mul=" << op1.mul << " add=" << op1.add << " rel=" << ry << " ms=" << tms1
            << (ok_ops1 ? "" : " [ops!]") << (ok_val1 ? "" : " [val!]") << "\n";
        all_ok &= ok1;

        // MatMul ok
        OpsCounter op2; Timer t2; t2.Tic(); Matrix C = A.Multiply(B, &op2); double tms2 = t2.TocMs();
        std::size_t mul2 = std::size_t(n) * n * n, add2 = std::size_t(n) * n * (n - 1);
        bool ok_ops2 = approx_eq(op2.mul, mul2, cfg.matmul_ops_tol) && approx_eq(op2.add, add2, cfg.matmul_ops_tol);
        Matrix Cref(n, n); LoadMatrixBin("datasets/C_" + std::to_string(n) + ".bin", Cref); Cref.rows = n; Cref.cols = n;
        double rC = rel_err_mat(C, Cref);
        bool ok_val2 = (rC <= 1e-12);
        bool ok2 = ok_ops2 && ok_val2;
        std::cout << "[Ex1][MatMul][n=" << n << "] " << (ok2 ? G : R) << (ok2 ? "PASS" : "FAIL") << Z
            << " mul=" << op2.mul << " add=" << op2.add << " relF=" << rC << " ms=" << tms2
            << (ok_ops2 ? "" : " [ops!]") << (ok_val2 ? "" : " [val!]") << "\n";
        all_ok &= ok2;

        // MatVec DIM MISMATCH (x_bad)
        bool pass_bad_mv = false;
        std::string msgMatVec;
        try {
            Vec x_bad; LoadVectorBin("datasets/x_bad_" + std::to_string(n) + ".bin", x_bad);
            (void)A.Multiply(x_bad, nullptr);  // hauria de llançar
        }
        catch (const std::invalid_argument& arg) { pass_bad_mv = true; msgMatVec = arg.what(); }
        std::cout << "[Ex1][MatVec-DIM][n=" << n << "] " << (pass_bad_mv ? G : R) << (pass_bad_mv ? "PASS" : "FAIL") << Z << " " << msgMatVec << "\n";
        all_ok &= pass_bad_mv;

        // MatMul DIM MISMATCH (B_bad)
        bool pass_bad_mm = false;
		std::string msgMatMul;
        try {
            Matrix B_bad; LoadMatrixBin("datasets/B_bad_" + std::to_string(n) + ".bin", B_bad);
            B_bad.rows = n - 1;
            B_bad.cols = n;
            (void)A.Multiply(B_bad, nullptr);  // s’espera std::invalid_argument

        }
        catch (const std::invalid_argument& arg) { pass_bad_mm = true; msgMatMul = arg.what(); }
        std::cout << "[Ex1][MatMul-DIM][n=" << n << "] " << (pass_bad_mm ? G : R) << (pass_bad_mm ? "PASS" : "FAIL") << Z << " " << msgMatMul << "\n";
        all_ok &= pass_bad_mm;
    }

    // ===== Ex2: Solve (No Pivot) — casos: OK i ZeroPivot/Singular =====
    for (int n : ns) {
        Matrix A(n, n); LoadMatrixBin("datasets/A_" + std::to_string(n) + ".bin", A); A.rows = n; A.cols = n;
        Vec rhs; LoadVectorBin("datasets/rhs_" + std::to_string(n) + ".bin", rhs);

        auto r_ok = LinAlg::SolveNoPivot(A, rhs, cfg.tol);
        double rel_ok = (r_ok.pivot_zero || r_ok.x.empty()) ? INFINITY : LinAlg::RelativeResidual(A, r_ok.x, rhs, nullptr);
        bool pass_ok = (!r_ok.pivot_zero) && (rel_ok <= 1e-8);
        std::cout << "[Ex2][OK][n=" << n << "] " << (pass_ok ? G : R) << (pass_ok ? "PASS" : "FAIL") << Z << " rel=" << rel_ok << " ms=" << r_ok.ms << "\n";
        all_ok &= pass_ok;

        Matrix Az; LoadMatrixBin("datasets/A_zeropiv_" + std::to_string(n) + ".bin", Az); Az.rows = n; Az.cols = n;
        Vec rhs_z; LoadVectorBin("datasets/rhs_zeropiv_" + std::to_string(n) + ".bin", rhs_z);
        auto r_z = LinAlg::SolveNoPivot(Az, rhs_z, cfg.tol);
        bool pass_z = r_z.pivot_zero;  // amb no-pivot ha de detectar pivot ~ 0
        std::cout << "[Ex2][ZeroPivot][n=" << n << "] " << (pass_z ? G : R) << (pass_z ? "PASS" : "FAIL") << Z << "\n";
        all_ok &= pass_z;

        Matrix As; LoadMatrixBin("datasets/A_sing_" + std::to_string(n) + ".bin", As); As.rows = n; As.cols = n;
        Vec rhs_s; LoadVectorBin("datasets/rhs_sing_" + std::to_string(n) + ".bin", rhs_s);
        auto r_s = LinAlg::SolveNoPivot(As, rhs_s, cfg.tol);
		bool pass_s = r_s.pivot_zero;  // singularitat => pivot ~ 0
        std::cout << "[Ex2][Singular][n=" << n << "] " << (pass_s ? G : R) << (pass_s ? "PASS" : "FAIL") << Z << "\n";
        all_ok &= pass_s;
    }

    // ===== Ex3: Solve (Partial Pivot) — ZeroPivot ha de resoldre; Singular ha de fallar =====
    for (int n : ns) {
        Matrix Az; LoadMatrixBin("datasets/A_zeropiv_" + std::to_string(n) + ".bin", Az); Az.rows = n; Az.cols = n;
        Vec rhs_z; LoadVectorBin("datasets/rhs_zeropiv_" + std::to_string(n) + ".bin", rhs_z);
        auto r_z = LinAlg::SolvePartialPivot(Az, rhs_z, cfg.tol);
        double rel_z = (r_z.singular || r_z.x.empty()) ? INFINITY : LinAlg::RelativeResidual(Az, r_z.x, rhs_z, nullptr);
        bool pass_z = (!r_z.singular) && (rel_z <= 1e-8);
        std::cout << "[Ex3][ZeroPivot][n=" << n << "] " << (pass_z ? G : R) << (pass_z ? "PASS" : "FAIL") << Z << " rel=" << rel_z << " ms=" << r_z.ms << "\n";
        all_ok &= pass_z;

        Matrix As; LoadMatrixBin("datasets/A_sing_" + std::to_string(n) + ".bin", As); As.rows = n; As.cols = n;
        Vec rhs_s; LoadVectorBin("datasets/rhs_sing_" + std::to_string(n) + ".bin", rhs_s);
        auto r_s = LinAlg::SolvePartialPivot(As, rhs_s, cfg.tol);
        bool pass_s = r_s.singular;  // singularitat ha de propagar
        std::cout << "[Ex3][Singular][n=" << n << "] " << (pass_s ? G : R) << (pass_s ? "PASS" : "FAIL") << Z << "\n";
        all_ok &= pass_s;
    }

    return all_ok ? 0 : 1;
}