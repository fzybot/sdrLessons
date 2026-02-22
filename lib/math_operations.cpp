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
                double summ_real = 0.0;
                double summ_imag = 0.0;
                for (int m = 0; m < b.size(); m++)
                {
                    // TODO: multiply not working
                    summ_real += a[i+m].real();// * b[j - m];
                    summ_imag += a[i+m].imag();// * b[j - m];
                }
                result[i + j] = {summ_real, summ_imag};
            }
        }
    }
    return result;
}

/*
Test set from Python NumPy
import numpy as np
import matplotlib.pyplot as plt
x = np.linspace(-4, 4, 41)
np.sinc(x)
array([-3.89804309e-17,  -4.92362781e-02,  -8.40918587e-02, # may vary
        -8.90384387e-02,  -5.84680802e-02,   3.89804309e-17,
        6.68206631e-02,   1.16434881e-01,   1.26137788e-01,
        8.50444803e-02,  -3.89804309e-17,  -1.03943254e-01,
        -1.89206682e-01,  -2.16236208e-01,  -1.55914881e-01,
        3.89804309e-17,   2.33872321e-01,   5.04551152e-01,
        7.56826729e-01,   9.35489284e-01,   1.00000000e+00,
        9.35489284e-01,   7.56826729e-01,   5.04551152e-01,
        2.33872321e-01,   3.89804309e-17,  -1.55914881e-01,
       -2.16236208e-01,  -1.89206682e-01,  -1.03943254e-01,
       -3.89804309e-17,   8.50444803e-02,   1.26137788e-01,
        1.16434881e-01,   6.68206631e-02,   3.89804309e-17,
        -5.84680802e-02,  -8.90384387e-02,  -8.40918587e-02,
        -4.92362781e-02,  -3.89804309e-17])

This func result:
        {-3.89817e-17 -0.0492363 -0.0840919 
        -0.0890384 -0.0584681 3.89817e-17 
        0.0668207 0.116435 0.126138 
        0.0850445 -3.89817e-17 -0.103943 
        -0.189207 -0.216236 -0.155915 3.89817e-17 
        0.233872 0.504551 0.756827 0.935489 1 0.935489 
        0.756827 0.504551 0.233872 3.89817e-17 
        -0.155915 -0.216236 -0.189207 -0.103943 
        -3.89817e-17 0.0850445 0.126138 0.116435 
        0.0668207 3.89817e-17 -0.0584681 -0.0890384 
        -0.0840919 -0.0492363 -3.89817e-17}
*/
std::vector<double> sinc(const std::vector<double>& x) {
    std::vector<double> y(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        double val = (x[i] == 0) ? 1.0e-20 : x[i];
        y[i] = M_PI * val;
    }
    std::vector<double> result(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = std::sin(y[i]) / y[i];
    }
    return result;
}

std::vector<double> linspace(double start, double end, int num) {
    std::vector<double> linspaced;
    if (num == 0) { return linspaced; }
    if (num == 1) {
        linspaced.push_back(start);
        return linspaced;
    }
    double delta = (end - start) / (num - 1);
    for (int i = 0; i < num; ++i) {
        linspaced.push_back(start + delta * i);
    }
    return linspaced;
}