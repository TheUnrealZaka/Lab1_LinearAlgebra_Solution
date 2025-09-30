#pragma once
#include "Matrix.hpp"
#include <string>

bool LoadMatrixBin(const std::string& path, Matrix& A);   // row-major doubles
bool LoadVectorBin(const std::string& path, Vec& v);
bool SaveMatrixBin(const std::string& path, const Matrix& A);
bool SaveVectorBin(const std::string& path, const Vec& v);