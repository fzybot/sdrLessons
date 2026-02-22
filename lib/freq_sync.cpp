#include <iostream>
#include <vector>
#include <complex.h>
#include <numbers> 

#include "math_operations.h"

std::vector<std::complex<double>> costas_loop(std::vector<std::complex<double>> &in)
{
    std::vector<std::complex<double>> iq_corr;
    iq_corr.resize(in.size());
    double pi_val = std::numbers::pi;
    double K1, K2, omega, phi = 0.0;
    double BnTs = 0.00001;
    int Nsps = 1;
    double Kp = 0.00065;
    double zeta = sqrt(2) / 2;

    double theta = (BnTs / Nsps) / (zeta + (0.25 / zeta));
    K1 = -4 * zeta * theta / ( (1 + 2 * zeta * theta + pow(theta,2)) * Kp);
    K2 = -4 * pow(theta,2) / ( (1 + 2 * zeta * theta + pow(theta,2))* Kp);
    int tau = 0;

    const std::complex<double> imag(0.0, 1.0);

    double error;
    for (int i = 0; i < in.size(); i++)
    {
        iq_corr[i] = in[i] * std::exp(imag * omega);
        //err ← sign(Re(IQ_corr[i]))Im(IQ_corr[i])-sign(Im(IQ_corr[i]))Re(IQ_corr[i])
        error = sgn(iq_corr[i].real()) * iq_corr[i].imag() - sgn(iq_corr[i].imag()) * iq_corr[i].real();
        phi = error * K1;
        while(omega >= 2* pi_val){
            omega = omega - 2 * pi_val;
        }
        while(omega < 0){
            omega = omega + 2 * pi_val;
        }
    }
    return iq_corr;
}