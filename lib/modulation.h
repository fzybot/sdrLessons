#include <stdint.h>
#include <complex.h>
#include <vector>

// For PlutoSDR
void bpsk(std::vector<int> &in, std::vector<std::complex<double>> &out);
void qpsk(std::vector<int> &in, std::vector<std::complex<double>> &out);

std::vector<std::complex<double>> modulate(std::vector<int> &in, int modulation_order);
