#pragma once
#include "Matrix.hpp"
#include "OpsCounter.hpp"

namespace LinAlg 
{
	double L2Norm(const Vec& v, OpsCounter* op = nullptr);  // TODO (Ex2)
	double RelativeResidual(const Matrix& A, const Vec& x, const Vec& b, OpsCounter* op = nullptr); // TODO (Ex2)
}