#pragma once
#include <cstddef>

struct BenchConfig 
{
	// operation thresholds
	double matvec_ops_tol = 0.02;
	double matmul_ops_tol = 0.02;
	double ge_ops_tol = 0.10;
	double back_ops_tol = 0.10;

	// scales
	double scale_min = 5.0;
	double scale_max = 12.0;

	// numeric tolerance
	double tol = 1e-12;
};