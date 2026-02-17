#include <stdint.h>
#include <complex.h>
#include <vector>

// For PlutoSDR
void bpsk(std::vector<int> &in, std::vector<std::complex<float>> &out);

std::vector<std::complex<float>> modulate(std::vector<int> &in, int modulation_order);
