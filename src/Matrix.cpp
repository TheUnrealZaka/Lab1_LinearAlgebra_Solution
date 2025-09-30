#include "Matrix.hpp"
#include <stdexcept>

Matrix Matrix::Identity(std::size_t n) 
{
    // TODO (Ex1)
    
}

double& Matrix::At(std::size_t i, std::size_t j) 
{
    // TODO (Ex1) (opcional: assert bounds)
    
}
double Matrix::At(std::size_t i, std::size_t j) const 
{
    // TODO (Ex1) (opcional: assert bounds)
}

Vec Matrix::Multiply(const Vec& x, OpsCounter* op) const 
{
    // TODO (Ex1): A (m×n) * x (n)
    
}

Matrix Matrix::Multiply(const Matrix& B, OpsCounter* op) const 
{
    // TODO (Ex1): A (m×n) * B (n×p)
    
}
