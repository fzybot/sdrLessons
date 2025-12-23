#include <vector>
#include <iostream>
#include <complex.h>

std::vector<std::complex<double>> upsample(std::vector<std::complex<double>> &vec, int samples_per_symbol)
{
    std::vector<std::complex<double>> upsampled;
    upsampled.resize(vec.size() * samples_per_symbol);
    int j = 0;
    for (int i = 0; i < upsampled.size(); i += samples_per_symbol)
    {
        upsampled[i] = vec[j];
        j++;
    }
    return upsampled;
}