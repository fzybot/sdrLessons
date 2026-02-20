#include <vector>
#include <cstdint>
#include <iostream>
#include <complex.h>

void bpsk(std::vector<int> &in, std::vector<std::complex<double>> &out)
{
    std::complex<double> val;
    for (int i = 0; i < in.size(); i++)
    {
        if(in[i] == 1){
            val = 1.0 + 0j;
        } else {
            val = -1.0 + 0j;
        }
        out.push_back(val);
    }
}

void qpsk(std::vector<int> &in, std::vector<std::complex<double>> &out)
{
    std::complex<double> val;
    double real, imag, k;
    out.resize(in.size() / 2);
    k = 1 / sqrt(2);
    int j = 0;
    for (int i = 0; i < in.size(); i += 2)
    {
        real = k * (1 - 2 * in[i]);
        imag = k * (1 - 2 * in[i + 1]);
        out[j] = std::complex<double>(real, imag);
        j++;
    }
}


std::vector<std::complex<double>> modulate(std::vector<int> &in, int modulation_order)
{
    std::vector<std::complex<double>> iq_samples;
    switch(modulation_order){
        case 1:
            bpsk(in, iq_samples);
            break;
        case 2:
            if(in.size() % 2 != 0){
                std::cout << "Error, array % 2 != 0" << std::endl;
                return iq_samples;
            }
            else {
                qpsk(in, iq_samples);
            }
            break;
    }

    return iq_samples;
}