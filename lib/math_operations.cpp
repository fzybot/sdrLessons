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
    // for (int i = 0; i < a.size(); i += 1){
    //     float summ_real = 0.0;
    //     float summ_imag = 0.0;
    //     for (int m = 0; m < b.size(); m++)
    //     {
    //         summ_real += a[i*m].real() * b[i - m];
    //         summ_imag += a[i*m].imag() * b[i - m];
    //     }
    //     std::complex<float> complex_num(summ_real, summ_imag);

    //     result.push_back(complex_num);
    // }

    return result;
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