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

    for (int i = 0; i < a.size(); i += 1){
        double summ_real = 0.0;
        double summ_imag = 0.0;
        for (int m = 0; m < b.size(); m++)
        {
            summ_real += a[i*m].real() * b[j - m];
            summ_imag += a[i*m].imag() * b[j - m];
        }
        std::complex<double> complex_num(summ_real, summ_imag);

        result.push_back(complex_num);
    }

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