#include <vector>
#include <cstdint>
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

std::vector<std::complex<double>> modulate(std::vector<int> &in, int modulation_order)
{
    std::vector<std::complex<double>> iq_samples;
    switch(modulation_order){
        case 1:
            bpsk(in, iq_samples);
            break;
        case 2:
            break;
    }

    return iq_samples;
}