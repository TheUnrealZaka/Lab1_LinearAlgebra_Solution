#include "Solve.hpp"
#include "LinAlg.hpp"
#include <stdexcept>
#include <cmath>

namespace LinAlg 
{

    bool GaussianElimination(Matrix& A, Vec& b, double tol, OpsCounter* op) 
    {
        // TODO (Ex2)
    }

    void BackSubstitution(const Matrix& U, Vec& c, OpsCounter* op) 
    {
        // TODO (Ex2)
    }

    SolveReport SolveNoPivot(Matrix A, Vec b, double tol) 
    {
        // TODO (Ex2)
    }

    bool GaussianEliminationPivot(Matrix& A, Vec& b, double tol, OpsCounter* op) 
    {
        // TODO (Ex3)
    }


    SolveReport SolvePartialPivot(Matrix A, Vec b, double tol) 
    {
        // TODO (Ex3)
    }
}
