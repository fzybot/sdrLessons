#include <vector>
#include <complex.h>

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

std::vector<std::complex<double>> convolve(std::vector<std::complex<double>> &a, std::vector<double> &b);

std::vector<double> convolve(std::vector<double> &a, std::vector<double> &b);
std::vector<double> sinc(const std::vector<double> &x);
std::vector<double> linspace(double start, double end, int num);