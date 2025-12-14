#include <iostream>
#include <vector>

std::vector<float> convolve(std::vector<float> &a, std::vector<float> &b){
    
    std::vector<float> result;
    for(int n = 0; n < a.size(); n++)
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