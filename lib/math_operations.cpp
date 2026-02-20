#include <iostream>
#include <vector>
#include <complex.h>

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


    for (int i = 0; i < matched.size() ; i+=samples_per_symbol)
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