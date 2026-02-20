#include <vector>
#include <iostream>
#include <complex.h>
#include "math_operations.h"

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

std::vector<std::complex<double>> pulse_shaping(std::vector<std::complex<double>> &vec, int type, int sps)
{
    std::vector<std::complex<double>> iq_samples;
    switch(type){
        case 0:
            std::vector<double> filter;
            filter.resize(sps);
            for (int i = 0; i < filter.size(); i++)
            {
                filter[i] = 1.0;
                std::cout << filter[i] << " ";
            }
            std::cout << std::endl;
            iq_samples = convolve(vec, filter);
            break;
    }

    return iq_samples;
}