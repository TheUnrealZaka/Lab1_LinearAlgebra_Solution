#pragma once
#include "Matrix.hpp"
#include "OpsCounter.hpp"
#include "Timer.hpp"

namespace LinAlg 
{

	struct SolveReport 
	{
		Vec x;
		bool pivot_zero = false;
		bool singular = false;
		std::size_t n = 0;
		OpsCounter ops{};
		double ms = 0.0;
		double rel_resid = 0.0;
	};

	bool GaussianElimination(Matrix& A, Vec& b, double tol, OpsCounter* op = nullptr);		// TODO (Ex2)
	void BackSubstitution(const Matrix& U, Vec& c, OpsCounter* op = nullptr);				// TODO (Ex2)
	SolveReport SolveNoPivot(Matrix A, Vec b, double tol);									// TODO (Ex2)

	bool GaussianEliminationPivot(Matrix& A, Vec& b, double tol, OpsCounter* op = nullptr); // TODO (Ex3)
	SolveReport SolvePartialPivot(Matrix A, Vec b, double tol);								// TODO (Ex3)
}