#include <iostream>
#include <vector>
#include <complex.h>
#include <numbers> 

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


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

std::vector<int> ted(std::vector<std::complex<double>> &matched, int samples_per_symbol)
{
    int K1, K2, p1, p2 = 0;
    double BnTs = 0.001;
    double Kp = 0.002;
    double zeta = sqrt(2) / 2;
    double theta = (BnTs / samples_per_symbol) / (zeta + (0.25 / zeta));
    K1 = -4 * zeta * theta / ( (1 + 2 * zeta * theta + pow(theta,2)) * Kp);
    K2 = -4 * pow(theta,2) / ( (1 + 2 * zeta * theta + pow(theta,2))* Kp);
    int tau = 0;
    double err;
    std::vector<int> errof;

    errof.push_back(0);
    for (int i = 0; i < matched.size() - samples_per_symbol; i += samples_per_symbol)
    {
        err = (matched[i + samples_per_symbol + tau].real() - matched[i + tau]).real() * matched[i + (samples_per_symbol / 2) + tau].real() + (matched[i + samples_per_symbol + tau].imag() - matched[i + tau]).imag() * matched[i + (samples_per_symbol / 2) + tau].imag(); 
        p1 = err * K1;
        p2 =  p2 + p1 + err * K2;

        if (p2 > 1)
        {
            p2 = p2 - 1;
        }

        if (p2 < -1)
        {
            p2 = p2 + 1;
        }
        tau = round(p2 * samples_per_symbol);
        errof.push_back(i + samples_per_symbol + tau);
    }

    return errof;
}

std::vector<std::complex<double>> symbol_sync(std::vector<std::complex<double>> &matched, int nsps)
{
    std::vector<int> ted_idxs = ted(matched, nsps);
    // for (int i = 0; i < ted_idxs.size(); i++){
    //     std::cout << ted_idxs[i] << " ";
    // }
    // std::cout << "" << std::endl;

    std::vector<std::complex<double>> symb_samples;
    symb_samples.resize(ted_idxs.size());
    for (int i = 0; i < ted_idxs.size(); i++)
    {
        symb_samples[i] = matched[ted_idxs[i]];
    }
    return symb_samples;
}

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
// int main()
// {
//     std::cout<<"Hello World" << std::endl;
//     std::vector<double> a = {1, 2, 3};
//     std::vector<double> b = {0, 1, 0.5};
//     std::vector<double> res = convolve(a,b);
//     for(int i = 0; i < res.size(); i++){
//         std::cout << res[i] << std::endl;
//     }
    
//     return 0;
// }