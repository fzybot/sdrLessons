#include <vector>
#include <complex.h>

std::vector<double> convolve(std::vector<double> &a, std::vector<double> &b);

std::vector<std::complex<double>> convolve(std::vector<std::complex<double>> &a, std::vector<double> &b);

std::vector<int> ted(std::vector<std::complex<double>> &a, int sps);
std::vector<std::complex<double>> symbol_sync(std::vector<std::complex<double>> &matched, int nsps);
std::vector<std::complex<double>> costas_loop(std::vector<std::complex<double>> &in);