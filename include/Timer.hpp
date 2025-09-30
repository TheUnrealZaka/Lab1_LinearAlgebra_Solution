#pragma once
#include <chrono>
struct Timer 
{
    using clk = std::chrono::steady_clock;
    clk::time_point t0;
    void Tic() { t0 = clk::now(); }
    double TocMs() const 
    {
        return std::chrono::duration<double, std::milli>(clk::now() - t0).count();
    }
};