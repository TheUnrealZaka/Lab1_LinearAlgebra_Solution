#include "LinAlg.hpp"
#include <cmath>
#include <stdexcept>

namespace LinAlg 
{

    double L2Norm(const Vec& v, OpsCounter* op) 
    {
        // Acumulador de la suma dels quadrats de cada component del vector.
        double sum = 0.0;

        // Recorrem tots els elements i apliquem la definició ||v||₂ = sqrt(∑ vᵢ²).
        for (std::size_t i = 0; i < v.size(); ++i) {
            sum += v[i] * v[i];

            // Actualitzem el comptador d'operacions si l'usuari l'ha proporcionat.
            if (op) {
                op->IncMul();  // Multiplicació v[i] * v[i]
                if (i > 0) {   // La primera addició no es compta perquè sum és zero
                    op->IncAdd();
                }
            }
        }

        // L'arrel quadrada final no es considera una operació elemental al comptador.
        return std::sqrt(sum);
    }

    double RelativeResidual(const Matrix& A, const Vec& x, const Vec& b, OpsCounter* op) 
    {
        // Residual relatiu definit com ||Ax - b||₂ / ||b||₂.

        // Pas 1: construïm el producte Ax.
        Vec Ax = A.Multiply(x, op);

        // Pas 2: residu r = Ax - b treballant sobre una còpia de Ax.
        Vec r = Ax;
        for (std::size_t i = 0; i < r.size(); ++i) {
            r[i] -= b[i];
            if (op) {
                op->IncSub();  // Resta Ax[i] - b[i]
            }
        }

        // Pas 3: normalitzem calculant les normes ||r||₂ i ||b||₂.
        double norm_r = L2Norm(r, op);
        double norm_b = L2Norm(b, op);

        // Pas 4: si ||b||₂ és nul, retornem ||r||₂ per evitar divisions per zero.
        if (norm_b == 0.0) {
            return norm_r;
        }

        // Comptem l'operació de divisió final si hi ha comptador.
        if (op) {
            op->IncDiv();
        }

        return norm_r / norm_b;
    }

}
