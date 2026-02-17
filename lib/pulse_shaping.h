#include <vector>
#include <complex.h>

std::vector<std::complex<float>> pulse_shaping(std::vector<std::complex<float>> &vec, int type, int sps);
std::vector<std::complex<float>> upsample(std::vector<std::complex<float>> &vec, int samples_per_symbol);