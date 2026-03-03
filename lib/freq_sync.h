#include <vector>
#include <complex.h>


std::vector<std::complex<double>> coarse_freq_sync(std::vector<std::complex<double>> &in);
std::vector<std::complex<double>> costas_loop(std::vector<std::complex<double>> &in);
std::vector<std::complex<double>> costas_loop_bpsk(const std::vector<std::complex<double>>& samples);