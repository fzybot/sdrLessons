#include <vector>
#include <complex.h>

std::vector<float> convolve(std::vector<float> &a, std::vector<float> &b);

std::vector<std::complex<float>> convolve(std::vector<std::complex<float>> &a, std::vector<float> &b);

std::vector<float> ted(std::vector<std::complex<float>> &a, int sps);