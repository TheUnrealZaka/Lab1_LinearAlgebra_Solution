#include "LinAlg.hpp"
#include <cmath>
#include <stdexcept>

/**
 * EXERCICI 2: Operacions auxiliars per a l'àlgebra lineal
 * 
 * Aquest fitxer conté funcions auxiliars utilitzades per calcular normes
 * i residus relatius, necessàries per verificar la precisió de les solucions
 * dels sistemes lineals.
 */

namespace LinAlg 
{

// ============================================================================
// FUNCIÓ: L2Norm
// ============================================================================
/**
 * Calcula la norma euclidiana (norma L2) d'un vector.
 * 
 * La norma L2 mesura la "longitud" d'un vector en l'espai euclidià:
 *     ||v||₂ = √(v₁² + v₂² + ... + vₙ²)
 * 
 * Exemple:
 *     Si v = (3, 4), aleshores ||v||₂ = √(3² + 4²) = √(9 + 16) = √25 = 5
 * 
 * Complexitat temporal: O(n)
 * Nombre d'operacions: n multiplicacions + (n-1) sumes
 * 
 * @param v: Vector del qual volem calcular la norma
 * @param op: Comptador d'operacions (opcional)
 * @return: Valor de la norma ||v||₂
 */
double L2Norm(const Vec& v, OpsCounter* op) 
{
    // Acumulador per la suma dels quadrats
    double sum = 0.0;

    // Recorrem tots els elements del vector
    for (std::size_t i = 0; i < v.size(); ++i) {
        // Sumem el quadrat de cada component: sum = sum + v[i]²
        sum += v[i] * v[i];

        // Comptatge d'operacions
        if (op) {
            op->IncMul();  // Comptem la multiplicació v[i] * v[i]
            
            // IMPORTANT: La primera "suma" no es compta perquè sum comença a 0
            // Només comptem sumes reals (quan i > 0)
            if (i > 0) {
                op->IncAdd();  // Comptem la suma sum += ...
            }
        }
    }

    // Retornem l'arrel quadrada de la suma
    // NOTA: sqrt() no es compta com a operació elemental (FLOP)
    return std::sqrt(sum);
}

// ============================================================================
// FUNCIÓ: RelativeResidual
// ============================================================================
/**
 * Calcula el residu relatiu d'una solució d'un sistema lineal.
 * 
 * El residu relatiu mesura la precisió de la solució x al sistema Ax = b:
 *     residu_relatiu = ||Ax - b||₂ / ||b||₂
 * 
 * Un residu relatiu petit (per exemple, < 10⁻¹⁰) indica que x és una bona
 * aproximació de la solució real.
 * 
 * Interpretació:
 *     - residu ≈ 10⁻¹²: Solució molt precisa (errors de màquina)
 *     - residu ≈ 10⁻⁶:  Solució acceptable per a moltes aplicacions
 *     - residu > 10⁻³:  Solució poc precisa, cal revisar
 * 
 * @param A: Matriu del sistema
 * @param x: Vector solució a verificar
 * @param b: Vector de termes independents
 * @param op: Comptador d'operacions (opcional)
 * @return: Valor del residu relatiu
 */
double RelativeResidual(const Matrix& A, const Vec& x, const Vec& b, OpsCounter* op) 
{
    // PAS 1: Calculem el producte Ax
    // Això ens dona el que obtindríem si substituíssim x al sistema
    Vec Ax = A.Multiply(x, op);

    // PAS 2: Calculem el residu r = Ax - b
    // Aquest vector mesura l'error: hauria de ser ≈ 0 si x és solució exacta
    Vec r = Ax;  // Copiem Ax per treballar-hi
    
    for (std::size_t i = 0; i < r.size(); ++i) {
        r[i] -= b[i];  // r[i] = Ax[i] - b[i]
        if (op) {
            op->IncSub();  // Comptem 1 resta per cada component
        }
    }

    // PAS 3: Calculem les normes ||r||₂ i ||b||₂
    // La norma de r mesura la magnitud de l'error
    double norm_r = L2Norm(r, op);
    
    // La norma de b ens serveix per normalitzar (fer el residu "relatiu")
    double norm_b = L2Norm(b, op);

    // PAS 4: Cas especial: si b és el vector zero, no podem dividir
    if (norm_b == 0.0) {
        return norm_r;  // Retornem directament ||r||₂
    }

    // Comptem la divisió final
    if (op) {
        op->IncDiv();
    }

    // Retornem el residu relatiu = ||r||₂ / ||b||₂
    return norm_r / norm_b;
}

}  // namespace LinAlg
