#include "Solve.hpp"
#include "LinAlg.hpp"
#include <stdexcept>
#include <cmath>

/**
 * EXERCICI 2 i 3: Resolució de sistemes lineals Ax = b
 * 
 * Aquest fitxer implementa els algorismes d'eliminació gaussiana i substitució
 * enrere per resoldre sistemes d'equacions lineals, com els que apareixen
 * en el context del RPG quan volem combinar recursos per obtenir un objecte.
 * 
 * Context del problema:
 *     Ax = b, on:
 *     - A: matriu amb els atributs dels recursos (cada columna = un recurs)
 *     - x: quantitats de cada recurs a barrejar (incògnita)
 *     - b: atributs de l'objecte final desitjat
 */

namespace LinAlg 
{

// ============================================================================
// FUNCIÓ: GaussianElimination (EXERCICI 2)
// ============================================================================
/**
 * Aplica l'eliminació gaussiana SENSE pivotatge per transformar Ax = b en Ux = c.
 * 
 * L'eliminació gaussiana transforma el sistema en un de triangular superior:
 *     Abans:  [a₁₁ a₁₂ a₁₃ | b₁]    Després: [u₁₁ u₁₂ u₁₃ | c₁]
 *             [a₂₁ a₂₂ a₂₃ | b₂]  →          [ 0  u₂₂ u₂₃ | c₂]
 *             [a₃₁ a₃₂ a₃₃ | b₃]             [ 0   0  u₃₃ | c₃]
 * 
 * Algoritme per cada columna k:
 *     1. Calculem el multiplicador: m[i,k] = A[i,k] / A[k,k]  (per cada fila i > k)
 *     2. Actualitzem la fila i: A[i,j] = A[i,j] - m[i,k] * A[k,j]  (per cada j > k)
 *     3. Actualitzem b[i]: b[i] = b[i] - m[i,k] * b[k]
 * 
 * Complexitat temporal: O(n³)
 * Nombre d'operacions: ≈ n³/3 multiplicacions + n³/3 restes + n²/2 divisions
 * 
 * @param A: Matriu del sistema (ES MODIFICA: esdevé triangular superior)
 * @param b: Vector de termes independents (ES MODIFICA: esdevé c)
 * @param tol: Tolerància per detectar pivots massa petits (típicament 10⁻¹²)
 * @param op: Comptador d'operacions (opcional)
 * @return: true si l'eliminació té èxit, false si trobem un pivot ≈ 0
 */
bool GaussianElimination(Matrix& A, Vec& b, double tol, OpsCounter* op) 
{
    std::size_t n = A.rows;

    // VALIDACIÓ: Només funciona per matrius quadrades
    if (A.cols != n || b.size() != n) {
        return false;
    }
    
    // Cas especial: matriu buida
    if (n == 0) {
        return true;
    }

    // BUCLE PRINCIPAL: recorrem cada columna k (que serà el "pivot")
    for (std::size_t k = 0; k < n; ++k) {
        
        // Obtenim l'element pivot A[k,k]
        double pivot = A.At(k, k);

        // COMPROVACIÓ CRÍTICA: El pivot ha de ser prou gran (> tolerància)
        // Si és massa petit, el sistema podria ser singular o mal condicionat
        if (op) op->IncCmp();  // Comptem la comparació amb la tolerància
        
        if (std::abs(pivot) <= tol) {
            return false;  // FALLIDA: pivot massa petit, no podem continuar
        }

        // OPTIMITZACIÓ: Si som a l'última fila, ja hem acabat
        if (k == n - 1) {
            continue;
        }

        // ELIMINACIÓ: anul·lem tots els elements sota el pivot
        for (std::size_t i = k + 1; i < n; ++i) {
            
            // PAS 1: Calculem el multiplicador m[i,k] = A[i,k] / pivot
            double m_ik = A.At(i, k) / pivot;
            if (op) op->IncDiv();  // Comptem 1 divisió
            
            // IMPORTANT: Guardem m[i,k] a la posició A[i,k] (sota la diagonal)
            // Això no cal per resoldre el sistema, però estalvia memòria
            A.At(i, k) = m_ik;

            // PAS 2: Actualitzem la fila i de la matriu A
            // Per cada element a la dreta del pivot: A[i,j] = A[i,j] - m[i,k] * A[k,j]
            for (std::size_t j = k + 1; j < n; ++j) {
                A.At(i, j) -= m_ik * A.At(k, j);
                if (op) {
                    op->IncMul();  // Comptem 1 multiplicació: m[i,k] * A[k,j]
                    op->IncSub();  // Comptem 1 resta
                }
            }

            // PAS 3: Actualitzem també el terme independent b[i]
            b[i] -= m_ik * b[k];
            if (op) {
                op->IncMul();  // Comptem 1 multiplicació: m[i,k] * b[k]
                op->IncSub();  // Comptem 1 resta
            }
        }
    }

    return true;  // ÈXIT: la matriu és ara triangular superior
}

// ============================================================================
// FUNCIÓ: BackSubstitution (EXERCICI 2)
// ============================================================================
/**
 * Resol un sistema triangular superior Ux = c mitjançant substitució enrere.
 * 
 * Un cop tenim el sistema triangular superior després de l'eliminació gaussiana:
 *     [u₁₁ u₁₂ u₁₃] [x₁]   [c₁]
 *     [ 0  u₂₂ u₂₃] [x₂] = [c₂]
 *     [ 0   0  u₃₃] [x₃]   [c₃]
 * 
 * Algoritme (de baix a dalt):
 *     1. x[n-1] = c[n-1] / U[n-1,n-1]  (última equació)
 *     2. x[i] = (c[i] - Σ(U[i,j] * x[j])) / U[i,i]  (per i = n-2, ..., 0)
 * 
 * Exemple amb n=3:
 *     x₃ = c₃ / u₃₃
 *     x₂ = (c₂ - u₂₃·x₃) / u₂₂
 *     x₁ = (c₁ - u₁₂·x₂ - u₁₃·x₃) / u₁₁
 * 
 * Complexitat temporal: O(n²)
 * Nombre d'operacions: ≈ n²/2 multiplicacions + n²/2 restes + n divisions
 * 
 * @param U: Matriu triangular superior (NO es modifica)
 * @param c: Vector de termes independents (ES MODIFICA: esdevé la solució x)
 * @param op: Comptador d'operacions (opcional)
 * 
 * NOTA: Aquesta funció només es crida després de GaussianElimination, quan ja
 *       sabem que el sistema té solució, per tant NO cal comprovar U[i,i] == 0.
 */
void BackSubstitution(const Matrix& U, Vec& c, OpsCounter* op) 
{
    std::size_t n = U.rows;
    
    // VALIDACIÓ: dimensions correctes
    if (U.cols != n || c.size() != n) {
        throw std::invalid_argument("BackSubstitution: dimensions incompatibles");
    }
    
    // Cas especial: matriu buida
    if (n == 0) {
        return;
    }

    // PAS 1: Resolem l'última equació (fila n-1)
    // x[n-1] = c[n-1] / U[n-1,n-1]
    c[n - 1] /= U.At(n - 1, n - 1);
    if (op) op->IncDiv();  // Comptem 1 divisió

    // PAS 2: Resolem la resta d'equacions de baix a dalt
    // IMPORTANT: Usem int perquè anirem de n-2 fins a 0 (inclòs)
    for (int i = int(n) - 2; i >= 0; --i) {
        const std::size_t row = static_cast<std::size_t>(i);

        // Restem la contribució de les variables ja conegudes (x[j] amb j > i)
        // c[i] = c[i] - U[i,i+1]·x[i+1] - U[i,i+2]·x[i+2] - ... - U[i,n-1]·x[n-1]
        for (std::size_t j = row + 1; j < n; ++j) {
            c[row] -= U.At(row, j) * c[j];
            if (op) {
                op->IncMul();  // Comptem 1 multiplicació: U[row,j] * c[j]
                op->IncSub();  // Comptem 1 resta
            }
        }

        // Dividim pel coeficient diagonal per obtenir x[i]
        // x[i] = c[i] / U[i,i]
        c[row] /= U.At(row, row);
        if (op) op->IncDiv();  // Comptem 1 divisió
    }
    
    // RESULTAT: El vector c ara conté la solució x
}

// ============================================================================
// FUNCIÓ: SolveNoPivot (EXERCICI 2)
// ============================================================================
/**
 * Resol el sistema lineal Ax = b SENSE pivotatge.
 * 
 * Combina l'eliminació gaussiana i la substitució enrere per trobar la solució.
 * 
 * ATENCIÓ: Sense pivotatge, aquest mètode pot fallar si:
 *     - La matriu és singular (determinant = 0)
 *     - Hi ha pivots molt petits (problemes numèrics)
 * 
 * Per això és recomanable usar SolvePartialPivot (Exercici 3) en aplicacions reals.
 * 
 * @param A: Matriu del sistema (es passa per còpia, no es modifica l'original)
 * @param b: Vector de termes independents (es passa per còpia)
 * @param tol: Tolerància per considerar un pivot massa petit (ex: 10⁻¹²)
 * @return: Informe amb la solució, temps d'execució, operacions i residu
 */
SolveReport SolveNoPivot(Matrix A, Vec b, double tol) 
{
    // Creem l'informe que retornarem amb els resultats
    SolveReport report;
    report.n = A.rows;

    // VALIDACIÓ: només podem resoldre sistemes quadrats
    if (!A.IsSquare() || b.size() != A.rows) {
        return report;  // Retorna un informe buit indicant error
    }

    // IMPORTANT: Guardem còpies originals per calcular el residu després
    // (GaussianElimination modificarà A i b)
    Matrix A_orig = A;
    Vec b_orig = b;

    // Iniciem el cronòmetre per mesurar el temps d'execució
    Timer timer;
    timer.Tic();

    // FASE 1: Eliminació Gaussiana (transforma Ax = b en Ux = c)
    bool ge_success = GaussianElimination(A, b, tol, &report.ops);
    
    if (!ge_success) {
        // Si falla, indiquem que s'ha trobat un pivot ≈ 0
        report.pivot_zero = true;
        report.ms = timer.TocMs();
        return report;  // Retornem sense solució
    }

    // FASE 2: Substitució Enrere (resol Ux = c)
    BackSubstitution(A, b, &report.ops);

    // Guardem la solució (el vector b ara conté x)
    report.x = b;
    
    // Aturem el cronòmetre
    report.ms = timer.TocMs();

    // Calculem el residu relatiu ||Ax - b|| / ||b|| per verificar la precisió
    // NOTA: Usem nullptr perquè no volem comptar aquestes operacions
    report.rel_resid = RelativeResidual(A_orig, report.x, b_orig, nullptr);
    
    return report;
}

// ============================================================================
// FUNCIÓ: GaussianEliminationPivot (EXERCICI 3)
// ============================================================================
/**
 * Aplica l'eliminació gaussiana AMB pivotatge parcial.
 * 
 * El pivotatge parcial millora l'estabilitat numèrica de l'algorisme:
 *     - A cada etapa k, busquem el pivot MÉS GRAN (en valor absolut) a la columna k
 *     - Intercanviem files per portar aquest pivot a la diagonal
 *     - Això redueix els errors de precisió en aritmètica de coma flotant
 * 
 * Exemple d'intercanvi (k=0):
 *     Abans:  [1  2  3]    Després: [4  5  6]  (fila amb pivot més gran)
 *             [4  5  6]  →         [1  2  3]
 *             [7  8  9]             [7  8  9]
 * 
 * Diferències amb GaussianElimination:
 *     + Més estable numèricament
 *     + Pot resoldre més sistemes (els que fallaven sense pivotatge)
 *     - Lleugerament més lent (cerca del pivot + intercanvis)
 * 
 * Complexitat temporal: O(n³) (igual que sense pivotatge)
 * Operacions addicionals: n-1 cerques de pivot + intercanvis de files
 * 
 * @param A: Matriu del sistema (ES MODIFICA: esdevé triangular superior)
 * @param b: Vector de termes independents (ES MODIFICA: esdevé c)
 * @param tol: Tolerància per detectar matrius singulars
 * @param op: Comptador d'operacions (opcional)
 * @return: true si l'eliminació té èxit, false si la matriu és singular
 */
bool GaussianEliminationPivot(Matrix& A, Vec& b, double tol, OpsCounter* op) 
{
    std::size_t n = A.rows;
    
    // VALIDACIÓ: només funciona per matrius quadrades
    if (A.cols != n || b.size() != n) {
        return false;
    }
    
    // Cas especial: matriu buida
    if (n == 0) {
        return true;
    }

    // Accés directe a les dades per eficiència (molt utilitzat en aquesta funció)
    double* data = A.a.data();
    std::size_t ld = A.cols;  // Leading dimension (columnes)

    // BUCLE PRINCIPAL: recorrem cada columna k
    for (std::size_t k = 0; k < n; ++k) {
        
        // ====================================================================
        // FASE 1: SELECCIÓ DEL PIVOT (pivotatge parcial)
        // ====================================================================
        // Busquem la fila p >= k amb |A[p,k]| màxim
        
        std::size_t pivot_row = k;  // Inicialment assumim que el pivot és A[k,k]
        double max_pivot = std::abs(data[k * ld + k]);

        // Busquem a totes les files inferiors
        for (std::size_t p = k + 1; p < n; ++p) {
            double current_pivot = std::abs(data[p * ld + k]);
            
            // Comptem la comparació per trobar el màxim
            if (op) op->IncCmp();

            if (current_pivot > max_pivot) {
                max_pivot = current_pivot;
                pivot_row = p;  // Actualitzem la fila amb el pivot més gran
            }
        }

        // ====================================================================
        // FASE 2: INTERCANVI DE FILES (si cal)
        // ====================================================================
        if (pivot_row != k) {
            // Intercanviem les files k i pivot_row a la matriu A
            A.SwapRows(k, pivot_row);
            
            // També intercanviem els elements corresponents de b
            std::swap(b[k], b[pivot_row]);
            
            // IMPORTANT: Comptem 2 swaps (un per A, un per b)
            if (op) {
                op->IncSwp();  // Swap de la fila d'A
                op->IncSwp();  // Swap de l'element de b
            }
        }

        // ====================================================================
        // FASE 3: COMPROVACIÓ DEL PIVOT
        // ====================================================================
        double* row_k = data + k * ld;
        double pivot = row_k[k];
        
        // Comptem la comparació amb la tolerància
        if (op) op->IncCmp();
        
        // Si el pivot és massa petit, la matriu és singular
        if (std::abs(pivot) <= tol) {
            return false;  // FALLIDA: matriu singular
        }

        // OPTIMITZACIÓ: Si som a l'última fila, ja hem acabat
        if (k == n - 1) {
            continue;
        }

        // ====================================================================
        // FASE 4: ELIMINACIÓ GAUSSIANA (igual que sense pivotatge)
        // ====================================================================
        for (std::size_t i = k + 1; i < n; ++i) {
            double* row_i = data + i * ld;
            
            // Calculem el multiplicador
            double m_ik = row_i[k] / pivot;
            if (op) op->IncDiv();

            // Guardem el multiplicador sota la diagonal
            row_i[k] = m_ik;

            // Actualitzem la fila i de la matriu A
            for (std::size_t j = k + 1; j < ld; ++j) {
                row_i[j] -= m_ik * row_k[j];
                if (op) {
                    op->IncMul();
                    op->IncSub();
                }
            }

            // Actualitzem el vector b
            b[i] -= m_ik * b[k];
            if (op) {
                op->IncMul();
                op->IncSub();
            }
        }
    }

    return true;  // ÈXIT: la matriu és ara triangular superior
}


// ============================================================================
// FUNCIÓ: SolvePartialPivot (EXERCICI 3)
// ============================================================================
/**
 * Resol el sistema lineal Ax = b AMB pivotatge parcial.
 * 
 * Aquesta és la versió RECOMANADA per a aplicacions reals perquè:
 *     ✓ És numèricament estable
 *     ✓ Pot resoldre més sistemes que SolveNoPivot
 *     ✓ Detecta matrius singulars de manera fiable
 * 
 * Diferències amb SolveNoPivot:
 *     - Usa GaussianEliminationPivot en lloc de GaussianElimination
 *     - Lleugerament més lent (però més robust)
 *     - Marca report.singular en lloc de report.pivot_zero si falla
 * 
 * Exemple d'ús en el RPG:
 *     Si volem combinar recursos per obtenir un objecte amb atributs específics,
 *     aquest mètode ens dirà:
 *         - Les quantitats necessàries de cada recurs (si és possible)
 *         - O que la combinació és impossible (matriu singular)
 * 
 * @param A: Matriu del sistema (es passa per còpia)
 * @param b: Vector de termes independents (es passa per còpia)
 * @param tol: Tolerància per considerar la matriu singular
 * @return: Informe amb la solució, temps, operacions i residu
 */
SolveReport SolvePartialPivot(Matrix A, Vec b, double tol) 
{
    // Creem l'informe amb els resultats
    SolveReport report;
    report.n = A.rows;

    // VALIDACIÓ: només podem resoldre sistemes quadrats
    if (!A.IsSquare() || b.size() != A.rows) {
        return report;  // Retorna informe buit indicant error
    }

    // Guardem còpies originals per calcular el residu després
    Matrix A_orig = A;
    Vec b_orig = b;

    // Iniciem el cronòmetre
    Timer timer;
    timer.Tic();

    // FASE 1: Eliminació Gaussiana amb pivotatge parcial
    bool ge_success = GaussianEliminationPivot(A, b, tol, &report.ops);
    
    if (!ge_success) {
        // Si falla, la matriu és singular (determinant ≈ 0)
        report.singular = true;
        report.ms = timer.TocMs();
        return report;  // Retornem sense solució
    }

    // FASE 2: Substitució Enrere (igual que en SolveNoPivot)
    BackSubstitution(A, b, &report.ops);

    // Guardem la solució (b ara conté x)
    report.x = b;
    
    // Aturem el cronòmetre
    report.ms = timer.TocMs();

    // Calculem el residu relatiu per verificar la precisió
    report.rel_resid = RelativeResidual(A_orig, report.x, b_orig, nullptr);
    
    return report;
}

}  // namespace LinAlg
