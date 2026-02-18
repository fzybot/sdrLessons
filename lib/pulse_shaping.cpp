#include <vector>
#include <iostream>
#include <complex.h>
#include "math_operations.h"

std::vector<std::complex<float>> upsample(std::vector<std::complex<float>> &vec, int samples_per_symbol)
{
    std::vector<std::complex<float>> upsampled;
    upsampled.resize(vec.size() * samples_per_symbol);
    int j = 0;
    for (int i = 0; i < upsampled.size(); i += samples_per_symbol)
    {
        upsampled[i] = vec[j];
        j++;
    }
    return upsampled;
}

std::vector<std::complex<float>> pulse_shaping(std::vector<std::complex<float>> &vec, int type, int sps)
{
    std::vector<std::complex<float>> iq_samples;
    switch(type){
        case 0:
            std::vector<float> filter(sps, 1.0);
            for (int i = 0; i < filter.size(); i ++)
            {
                std::cout << filter[i] << " ";
            }
            std::cout << std::endl;
            iq_samples = convolve(vec, filter);
            break;
    }

    return iq_samples;
}