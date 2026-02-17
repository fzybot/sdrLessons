#include <iostream>
#include <vector>
#include <complex.h>

std::vector<float> convolve(std::vector<float> &a, std::vector<float> &b){
    
    std::vector<float> result;
    for(int n = 0; n < a.size(); n += 10)
    {
        float summ = 0;
        for(int m = 0; m < b.size(); m++)
        {
            summ += a[m] * b[n - m];
        }
        result.push_back(summ);
    }
    return result;
}

std::vector<std::complex<float>> convolve(std::vector<std::complex<float>> &a, std::vector<float> &b)
{
    std::vector<std::complex<float>> result;

    if(a.size() > b.size()){
        int N = b.size();
        for (int i = 0; i < a.size(); i += N){
            for (int j = 0; j < N; j++){
                float summ_real = 0.0;
                float summ_imag = 0.0;
                for (int m = 0; m < b.size(); m++)
                {
                    summ_real += a[i+m].real() * b[j - m];
                    summ_imag += a[i+m].imag() * b[j - m];
                }
                std::complex<float> complex_num(summ_real, summ_imag);

                result.push_back(complex_num);
            }
        }
    }
    return result;
}

std::vector<float> ted(std::vector<std::complex<float>> &matched, int samples_per_symbol)
{
    int K1, K2, p1, p2 = 0;
    int BnTs = 0.01;
    int Kp = 0.002;
    float zeta = sqrt(2) / 2;
    float theta = (BnTs / samples_per_symbol) / (zeta + (0.25 / zeta));
    K1 = -4 * zeta * theta / ( (1 + 2 * zeta * theta + pow(theta,2)) * Kp);
    K2 = -4 * pow(theta,2) / ( (1 + 2 * zeta * theta + pow(theta,2))* Kp);
    int tau = 0;
    float err;
    std::vector<float> errof;


    for (int i = 0; i < matched.size() - samples_per_symbol; i+=samples_per_symbol)
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
        tau = ceil(p2 * samples_per_symbol);
        errof.push_back(i + samples_per_symbol + tau);

    }
    return errof;
}
// int main()
// {
//     std::cout<<"Hello World" << std::endl;
//     std::vector<float> a = {1, 2, 3};
//     std::vector<float> b = {0, 1, 0.5};
//     std::vector<float> res = convolve(a,b);
//     for(int i = 0; i < res.size(); i++){
//         std::cout << res[i] << std::endl;
//     }
    
//     return 0;
// }