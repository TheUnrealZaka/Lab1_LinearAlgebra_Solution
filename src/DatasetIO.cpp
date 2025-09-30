#include "DatasetIO.hpp"
#include <fstream>
#include <filesystem>

static bool readAll(const std::string& path, std::vector<double>& out) 
{
	std::ifstream f(path, std::ios::binary);
	if (!f) return false;
	f.seekg(0, std::ios::end);
	std::streamsize N = f.tellg();
	if (N % sizeof(double) != 0) return false;
	f.seekg(0, std::ios::beg);
	out.resize(std::size_t(N / sizeof(double)));
	return bool(f.read(reinterpret_cast<char*>(out.data()), N));
}

bool LoadMatrixBin(const std::string& path, Matrix& A) 
{
	std::vector<double> raw;
	if (!readAll(path, raw)) return false;
	// filename A_n.bin with n*n elements
	A.a = std::move(raw);
	return true;
}

bool LoadVectorBin(const std::string& path, Vec& v) 
{
	std::vector<double> raw;
	if (!readAll(path, raw)) return false;
	v = std::move(raw);
	return true;
}

bool SaveMatrixBin(const std::string& path, const Matrix& A)
{
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);

    std::ofstream f(path, std::ios::binary | std::ios::trunc | std::ios::out);
    if (!f) return false;
    const std::streamsize bytes = static_cast<std::streamsize>(A.a.size() * sizeof(double));
    f.write(reinterpret_cast<const char*>(A.a.data()), bytes);
    return bool(f);
}

bool SaveVectorBin(const std::string& path, const Vec& v)
{
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);

    std::ofstream f(path, std::ios::binary | std::ios::trunc | std::ios::out);
    if (!f) return false;
    const std::streamsize bytes = static_cast<std::streamsize>(v.size() * sizeof(double));
    f.write(reinterpret_cast<const char*>(v.data()), bytes);
    return bool(f);
}