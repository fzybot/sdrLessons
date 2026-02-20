#include <vector>
#include <complex.h>

std::vector<std::complex<double>> pulse_shaping(std::vector<std::complex<double>> &vec, int type, int sps);
std::vector<std::complex<double>> upsample(std::vector<std::complex<double>> &vec, int samples_per_symbol);