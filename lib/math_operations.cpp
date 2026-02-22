#include <iostream>
#include <vector>
#include <complex.h>
#include <numbers> 

std::vector<double> convolve(std::vector<double> &a, std::vector<double> &b){
    
    std::vector<double> result;
    for(int n = 0; n < a.size(); n += 10)
    {
        double summ = 0;
        for(int m = 0; m < b.size(); m++)
        {
            summ += a[m] * b[n - m];
        }
        result.push_back(summ);
    }
    return result;
}

std::vector<std::complex<double>> convolve(std::vector<std::complex<double>> &a, std::vector<double> &b)
{
    std::vector<std::complex<double>> result;
    result.resize(a.size());
    std::complex<double> val;
    if (a.size() > b.size())
    {
        int N = b.size();
        for (int i = 0; i < a.size(); i += N){
            for (int j = 0; j < N; j++){
                double summ_real = 0.001;
                double summ_imag = 0.001;
                for (int m = 0; m < b.size(); m++)
                {
                    summ_real = summ_real +  a[i+m].real() * b[j - m];
                    summ_imag = summ_imag +  a[i+m].imag() * b[j - m];
                }
                result[i + j] = std::complex<double>(summ_real, summ_imag);
            }
        }
    }
    return result;
}