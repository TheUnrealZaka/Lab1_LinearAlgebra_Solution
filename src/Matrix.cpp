#include "Matrix.hpp"
#include <stdexcept>

/**
 * EXERCICI 1: Matrius i productes
 * 
 * Aquest fitxer implementa les operacions bàsiques sobre matrius necessàries
 * per resoldre sistemes lineals Ax = b que apareixen en el context de videojocs
 * (per exemple: combinar recursos amb diferents atributs).
 */

// ============================================================================
// FUNCIÓ: Matrix::Identity
// ============================================================================
/**
 * Crea una matriu identitat de dimensió n×n.
 * 
 * La matriu identitat és aquella que té 1's a la diagonal principal i 0's
 * a la resta de posicions:
 *     I = [1 0 0]
 *         [0 1 0]
 *         [0 0 1]
 * 
 * @param n: Dimensió de la matriu (nombre de files = nombre de columnes)
 * @return: Matriu identitat de dimensió n×n
 * 
 * Exemple:
 *     Matrix I = Matrix::Identity(3);  // Crea una matriu 3×3 identitat
 */
Matrix Matrix::Identity(std::size_t n) 
{
    // Creem una matriu quadrada n×n inicialitzada amb zeros
    Matrix I(n, n);
    
    // Recorrem només la diagonal principal i assignem 1's
    for (std::size_t i = 0; i < n; ++i) {
        I.At(i, i) = 1;  // At(i, i) accedeix a l'element de la fila i, columna i
    }
    
    return I;
}

// ============================================================================
// FUNCIÓ: Matrix::At (versió no-const)
// ============================================================================
/**
 * Accedeix a un element de la matriu per modificar-lo.
 * 
 * Les matrius s'emmagatzemen internament com un vector unidimensional en
 * format "row-major" (fila per fila). Per accedir a l'element (i, j):
 *     posició = i * nombre_columnes + j
 * 
 * @param i: Índex de la fila (comença en 0)
 * @param j: Índex de la columna (comença en 0)
 * @return: Referència a l'element A[i][j] (es pot modificar)
 * 
 * Exemple:
 *     Matrix A(3, 3);
 *     A.At(0, 1) = 5.0;  // Assigna el valor 5.0 a la fila 0, columna 1
 */
double& Matrix::At(std::size_t i, std::size_t j) 
{
    // Validació: comprovem que els índexs no surtin dels límits de la matriu
    if (i >= rows || j >= cols) {
        throw std::out_of_range("Matrix::At (non-const): índex fora de rang");
    }
    
    // Retornem una referència a l'element en la posició calculada
    return a[i * cols + j];
}

// ============================================================================
// FUNCIÓ: Matrix::At (versió const)
// ============================================================================
/**
 * Accedeix a un element de la matriu només per llegir-lo (sense modificar-lo).
 * 
 * Aquesta versió s'utilitza quan la matriu és constant (const Matrix).
 * 
 * @param i: Índex de la fila (comença en 0)
 * @param j: Índex de la columna (comença en 0)
 * @return: Valor de l'element A[i][j] (només lectura)
 */
double Matrix::At(std::size_t i, std::size_t j) const 
{
    // Validació dels índexs
    if (i >= rows || j >= cols) {
        throw std::out_of_range("Matrix::At (const): índex fora de rang");
    }
    
    // Retornem una còpia del valor (no es pot modificar)
    return a[i * cols + j];
}

// ============================================================================
// FUNCIÓ: Matrix::Multiply (producte matriu-vector)
// ============================================================================
/**
 * Calcula el producte matriu-vector: y = A·x
 * 
 * Exemple del RPG:
 *     Si A conté els atributs dels recursos i x les quantitats,
 *     el resultat y és l'objecte final amb els seus atributs combinats.
 * 
 * Matemàticament:
 *     Si A és m×n i x és n×1, aleshores y és m×1
 *     y[i] = A[i,0]·x[0] + A[i,1]·x[1] + ... + A[i,n-1]·x[n-1]
 * 
 * Complexitat temporal: O(m·n)
 * Nombre d'operacions: m·n multiplicacions + m·(n-1) sumes
 * 
 * @param x: Vector columna a multiplicar
 * @param op: Comptador d'operacions (opcional, per a l'anàlisi)
 * @return: Vector resultat y = A·x
 */
Vec Matrix::Multiply(const Vec& x, OpsCounter* op) const 
{
    // VALIDACIÓ: Les dimensions han de coincidir
    // El nombre de columnes d'A ha de ser igual al nombre d'elements de x
    if (cols != x.size()) {
        throw std::invalid_argument("Matrix::Multiply(mat-vec): dimensions incompatibles");
    }

    // Creem el vector resultat y de mida m (nombre de files)
    Vec y(rows);
    
    // Cas especial: matrius buides
    if (rows == 0 || cols == 0) {
        return y;
    }

    // Recorrem cada fila i de la matriu A
    for (std::size_t i = 0; i < rows; ++i) {
        const std::size_t row_offset = i * cols;  // Posició inicial de la fila i
        
        // OPTIMITZACIÓ: Calculem el primer terme fora del bucle
        // Això evita inicialitzar acc = 0 i fer una suma innecessària
        double acc = a[row_offset] * x[0];  // A[i,0] * x[0]
        if (op) op->IncMul();  // Comptem 1 multiplicació
        
        // Recorrem la resta d'elements de la fila
        for (std::size_t j = 1; j < cols; ++j) {
            acc += a[row_offset + j] * x[j];  // acc = acc + A[i,j] * x[j]
            if (op) {
                op->IncMul();  // Comptem 1 multiplicació
                op->IncAdd();  // Comptem 1 suma
            }
        }
        
        // Guardem el resultat a y[i]
        y[i] = acc;
    }

    return y;
}

// ============================================================================
// FUNCIÓ: Matrix::Multiply (producte matriu-matriu)
// ============================================================================
/**
 * Calcula el producte matriu-matriu: C = A·B
 * 
 * Matemàticament:
 *     Si A és m×n i B és n×p, aleshores C és m×p
 *     C[i,k] = A[i,0]·B[0,k] + A[i,1]·B[1,k] + ... + A[i,n-1]·B[n-1,k]
 * 
 * Complexitat temporal: O(m·n·p)
 * Nombre d'operacions: m·n·p multiplicacions + m·p·(n-1) sumes
 * 
 * @param B: Matriu a multiplicar
 * @param op: Comptador d'operacions (opcional, per a l'anàlisi)
 * @return: Matriu resultat C = A·B
 */
Matrix Matrix::Multiply(const Matrix& B, OpsCounter* op) const 
{
    // VALIDACIÓ: Les dimensions internes han de coincidir
    // El nombre de columnes d'A ha de ser igual al nombre de files de B
    if (cols != B.rows) {
        throw std::invalid_argument("Matrix::Multiply(mat-mat): dimensions incompatibles");
    }

    // Creem la matriu resultat C de dimensió m×p (files d'A × columnes de B)
    Matrix C(rows, B.cols);
    
    // Cas especial: matrius buides
    if (rows == 0 || cols == 0 || B.cols == 0) {
        return C;
    }

    // TRIPLE BUCLE: algorisme estàndard per multiplicar matrius
    
    // Bucle exterior: recorrem cada fila i de A
    for (std::size_t i = 0; i < rows; ++i) {
        const std::size_t a_row_offset = i * cols;  // Posició inicial de la fila i d'A
        
        // Bucle mitjà: recorrem cada columna k de B
        for (std::size_t k = 0; k < B.cols; ++k) {
            
            // OPTIMITZACIÓ: Calculem el primer terme fora del bucle interior
            double acc = a[a_row_offset + 0] * B.At(0, k);  // A[i,0] * B[0,k]
            if (op) op->IncMul();  // Comptem 1 multiplicació
            
            // Bucle interior: recorrem les dimensions internes (columnes d'A / files de B)
            for (std::size_t j = 1; j < cols; ++j) {
                // Producte escalar de la fila i d'A amb la columna k de B
                acc += a[a_row_offset + j] * B.At(j, k);  // acc = acc + A[i,j] * B[j,k]
                if (op) {
                    op->IncMul();  // Comptem 1 multiplicació
                    op->IncAdd();  // Comptem 1 suma
                }
            }
            
            // Guardem el resultat a C[i,k]
            C.At(i, k) = acc;
        }
    }

    return C;
}
