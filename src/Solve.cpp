#include "Solve.hpp"
#include "LinAlg.hpp"
#include <stdexcept>
#include <cmath>

namespace LinAlg 
{

    bool GaussianElimination(Matrix& A, Vec& b, double tol, OpsCounter* op) 
    {
        std::size_t n = A.rows;

        // Només té sentit per matrius quadrades amb el mateix nombre de components a b.
        if (A.cols != n || b.size() != n) {
            return false;
        }
        if (n == 0) {
            return true;
        }

        // Recorrem cada columna pivot k i anul·lem els elements per sota.
        for (std::size_t k = 0; k < n; ++k) {
            double pivot = A.At(k, k);

            // Comprovem la magnitud del pivot respecte la tolerància.
            if (op) op->IncCmp();
            if (std::abs(pivot) <= tol) {
                return false;  // Pivot massa petit: el procés sense pivotatge falla.
            }

            // Quan som a l'última fila no cal eliminar res per sota.
            if (k == n - 1) {
                continue;
            }

            // Eliminem les contribucions de la columna k a cada fila inferior i guardem el multiplicador.
            for (std::size_t i = k + 1; i < n; ++i) {
                double m_ik = A.At(i, k) / pivot;
                if (op) op->IncDiv();

                // Desem el factor d'eliminació a la part inferior de la matriu per reusar-lo a la substitució.
                A.At(i, k) = m_ik;

                // Actualitzem totes les entrades a la dreta del pivot a la fila i.
                for (std::size_t j = k + 1; j < n; ++j) {
                    A.At(i, j) -= m_ik * A.At(k, j);
                    if (op) {
                        op->IncMul();
                        op->IncSub();
                    }
                }

                // Actualitzem també el terme independent corresponent.
                b[i] -= m_ik * b[k];
                if (op) {
                    op->IncMul();
                    op->IncSub();
                }
            }
        }

        return true;
    }

    void BackSubstitution(const Matrix& U, Vec& c, OpsCounter* op) 
    {
        std::size_t n = U.rows;
        if (U.cols != n || c.size() != n) {
            throw std::invalid_argument("BackSubstitution: dimensions incompatibles");
        }
        if (n == 0) {
            return;
        }

        // Última fila
        c[n - 1] /= U.At(n - 1, n - 1);
        if (op) op->IncDiv();

        // Iterem de la penúltima fila fins a la primera
        for (int i = int(n) - 2; i >= 0; --i) {
            const std::size_t row = static_cast<std::size_t>(i);

            // Restem la contribució de les variables ja resoltes
            for (std::size_t j = row + 1; j < n; ++j) {
                c[row] -= U.At(row, j) * c[j];
                if (op) {
                    op->IncMul();
                    op->IncSub();
                }
            }

            // Dividim pel valor diagonal
            c[row] /= U.At(row, row);
            if (op) op->IncDiv();
        }
    }

    SolveReport SolveNoPivot(Matrix A, Vec b, double tol) 
    {
        SolveReport report;
        report.n = A.rows;

        // Validem dimensions: necessitem una matriu quadrada i un vector b compatible.
        if (!A.IsSquare() || b.size() != A.rows) {
            return report;
        }

        // Conservem còpies per poder calcular el residu amb les dades originals.
        Matrix A_orig = A;
        Vec b_orig = b;

        Timer timer;
        timer.Tic();

        // Fase 1: eliminació gaussiana sense pivotatge.
        bool ge_success = GaussianElimination(A, b, tol, &report.ops);
        if (!ge_success) {
            // Sense pivotatge el cas fallit indica un pivot massa petit.
            report.pivot_zero = true;
            report.ms = timer.TocMs();
            return report;
        }

        // Fase 2: substitució enrere utilitzant la part superior triangular d'A.
        BackSubstitution(A, b, &report.ops);

        // El vector b modificat ja conté la solució final.
        report.x = b;
        report.ms = timer.TocMs();

        // Mesurem el residu relatiu respecte les dades originals.
        report.rel_resid = RelativeResidual(A_orig, report.x, b_orig, nullptr);
        return report;
    }

    bool GaussianEliminationPivot(Matrix& A, Vec& b, double tol, OpsCounter* op) 
    {
        std::size_t n = A.rows;
        if (A.cols != n || b.size() != n) {
            return false;
        }
        if (n == 0) {
            return true;
        }

        double* data = A.a.data();
        std::size_t ld = A.cols;

        // Eliminació gaussiana amb pivotatge parcial fila a fila.
        for (std::size_t k = 0; k < n; ++k) {
            // 1) Selecció del pivot: busquem la fila amb |a[p, k]| més gran al submatriu restant.
            std::size_t pivot_row = k;
            double max_pivot = std::abs(data[k * ld + k]);

            for (std::size_t p = k + 1; p < n; ++p) {
                double current_pivot = std::abs(data[p * ld + k]);
                if (op) op->IncCmp();

                if (current_pivot > max_pivot) {
                    max_pivot = current_pivot;
                    pivot_row = p;
                }
            }

            // 2) Si cal, intercanviem la fila actual amb la candidata òptima.
            if (pivot_row != k) {
                A.SwapRows(k, pivot_row);
                std::swap(b[k], b[pivot_row]);
                // Comptem 2 swaps: un per A, un per b
                if (op) {
                    op->IncSwp();
                    op->IncSwp();
                }
            }

            double* row_k = data + k * ld;
            double pivot = row_k[k];
            if (op) op->IncCmp();
            if (std::abs(pivot) <= tol) {
                return false;  // Pivot massa petit: la matriu es considera singular.
            }

            if (k == n - 1) {
                continue;
            }

            // 3) A partir d'aquí, eliminació gaussiana estàndard reutilitzant el pivot triat.
            for (std::size_t i = k + 1; i < n; ++i) {
                double* row_i = data + i * ld;
                double m_ik = row_i[k] / pivot;
                if (op) op->IncDiv();

                // Guardem el multiplicador just a sota de la diagonal.
                row_i[k] = m_ik;

                // Actualitzem la resta d'elements a la fila i.
                for (std::size_t j = k + 1; j < ld; ++j) {
                    row_i[j] -= m_ik * row_k[j];
                    if (op) {
                        op->IncMul();
                        op->IncSub();
                    }
                }

                // També actualitzem el vector de termes independents.
                b[i] -= m_ik * b[k];
                if (op) {
                    op->IncMul();
                    op->IncSub();
                }
            }
        }

        return true;
    }


    SolveReport SolvePartialPivot(Matrix A, Vec b, double tol) 
    {
        SolveReport report;
        report.n = A.rows;

        // Validem que tinguem una matriu quadrada i un vector de la mateixa llargada.
        if (!A.IsSquare() || b.size() != A.rows) {
            return report;
        }

        // Desarem còpies independents per poder calcular el residu relatiu.
        Matrix A_orig = A;
        Vec b_orig = b;

        Timer timer;
        timer.Tic();

        // Fase 1: eliminació gaussiana amb pivotatge parcial fila a fila.
        bool ge_success = GaussianEliminationPivot(A, b, tol, &report.ops);
        if (!ge_success) {
            // Pivot massa petit: declarem que la matriu és singular.
            report.singular = true;
            report.ms = timer.TocMs();
            return report;
        }

        // Fase 2: un cop tenim U triangular superior, fem substitució enrere.
        BackSubstitution(A, b, &report.ops);

        // Guardem la solució, el temps i el residu relatiu.
        report.x = b;
        report.ms = timer.TocMs();

        report.rel_resid = RelativeResidual(A_orig, report.x, b_orig, nullptr);
        return report;
    }

    // Exercici 2: Resoldre amb matriu ampliada
    // El nombre de FLOPs és similar a SolvePartialPivot, però el temps és menor gràcies a la millor localitat de memòria: A i b són contigus, reduint els cache misses.
    SolveReport SolvePivotAugmented(Matrix Ab, double tol)
    {
        SolveReport report;
        
        // Validació: Ab ha de ser n x (n+1)
        if (Ab.cols != Ab.rows + 1) {
            throw std::invalid_argument("SolvePivotAugmented: Ab ha de ser n x (n+1)");
        }
        
        std::size_t n = Ab.rows;
        report.n = n;
        
        if (n == 0) {
            return report;
        }
        
        // Guardem còpies per calcular el residu relatiu
        Matrix A_orig(n, n, 0.0);
        Vec b_orig(n, 0.0);
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                A_orig.At(i, j) = Ab.At(i, j);
            }
            b_orig[i] = Ab.At(i, n);
        }
        
        Timer timer;
        timer.Tic();
        
        double* data = Ab.a.data();
        std::size_t ld = Ab.cols;  // n+1
        
        // Fase 1: Eliminació Gaussiana amb pivotatge parcial sobre la matriu ampliada
        for (std::size_t k = 0; k < n; ++k) {
            // Selecció del pivot: busquem la fila amb |a[p, k]| més gran
            std::size_t pivot_row = k;
            double max_pivot = std::abs(data[k * ld + k]);
            
            for (std::size_t p = k + 1; p < n; ++p) {
                double current_pivot = std::abs(data[p * ld + k]);
                report.ops.IncCmp();
                
                if (current_pivot > max_pivot) {
                    max_pivot = current_pivot;
                    pivot_row = p;
                }
            }
            
            // Intercanviem files si cal
            if (pivot_row != k) {
                Ab.SwapRows(k, pivot_row);
                // Comptem 2 swaps per consistència amb SolvePartialPivot (un per A, un per b)
                report.ops.IncSwp();
                report.ops.IncSwp();
            }
            
            double* row_k = data + k * ld;
            double pivot = row_k[k];
            report.ops.IncCmp();
            if (std::abs(pivot) <= tol) {
                // Pivot massa petit: matriu singular
                report.singular = true;
                report.ms = timer.TocMs();
                return report;
            }
            
            if (k == n - 1) {
                continue;
            }
            
            // Eliminació gaussiana sobre tota la fila ampliada
            for (std::size_t i = k + 1; i < n; ++i) {
                double* row_i = data + i * ld;
                double m_ik = row_i[k] / pivot;
                report.ops.IncDiv();
                
                // Guardem el multiplicador
                row_i[k] = m_ik;
                
                // Actualitzem la resta de la fila (inclou la darrera columna = b)
                for (std::size_t j = k + 1; j < ld; ++j) {
                    row_i[j] -= m_ik * row_k[j];
                    report.ops.IncMul();
                    report.ops.IncSub();
                }
            }
        }
        
        // Fase 2: Substitució enrere operant sobre la darrera columna d'Ab
        // La darrera columna (índex n) conté el vector b modificat
        
        // Última fila
        data[(n - 1) * ld + n] /= data[(n - 1) * ld + (n - 1)];
        report.ops.IncDiv();
        
        // Iterem de la penúltima fila fins a la primera
        for (int i = int(n) - 2; i >= 0; --i) {
            const std::size_t row = static_cast<std::size_t>(i);
            double* row_ptr = data + row * ld;
            
            // Restem la contribució de les variables ja resoltes
            for (std::size_t j = row + 1; j < n; ++j) {
                row_ptr[n] -= row_ptr[j] * data[j * ld + n];
                report.ops.IncMul();
                report.ops.IncSub();
            }
            
            // Dividim pel valor diagonal
            row_ptr[n] /= row_ptr[row];
            report.ops.IncDiv();
        }
        
        // Extraiem la solució de la darrera columna
        report.x.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            report.x[i] = data[i * ld + n];
        }
        
        report.ms = timer.TocMs();
        
        // Calculem el residu relatiu
        report.rel_resid = RelativeResidual(A_orig, report.x, b_orig, nullptr);
        
        return report;
    }
}
