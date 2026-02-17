#include <vector>
#include <cstdint>
#include <complex.h>

void bpsk(std::vector<int> &in, std::vector<std::complex<float>> &out)
{
    std::complex<float> val;
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


std::vector<std::complex<float>> modulate(std::vector<int> &in, int modulation_order)
{
    std::vector<std::complex<float>> iq_samples;
    switch(modulation_order){
        case 1:
            bpsk(in, iq_samples);
            break;
        case 2:
            break;
    }

    return iq_samples;
}