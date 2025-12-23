#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include "plots.h" 

namespace plt = matplotlibcpp;

void plot_vector(const std::vector<int> &y_vals, std::string title = "",
                 std::string x_lable = "",
                 std::string y_lable = "", 
                 bool last = false)
{
    int N = y_vals.size();
    std::vector<int> indexes(N);
    std::iota(std::begin(indexes), std::end(indexes), 0);

    plt::figure();
    plt::scatter(indexes, y_vals, 20.0);
    plt::title(title);
    plt::xlabel(x_lable);
    plt::ylabel(y_lable);
    plt::grid(true);
    if(last){
        plt::show();
    } else {
        plt::show(false);
    }
}

void plot_vector(const std::vector<std::complex<double>> &y_vals, std::string title = "",
                 std::string x_lable = "",
                 std::string y_lable = "", 
                 bool last = false)
{
    int N = y_vals.size();
    std::vector<int> indexes(N);
    std::iota(std::begin(indexes), std::end(indexes), 0);
    std::vector<int> real, imag;
    for (int i = 0; i < y_vals.size(); i++){
        real.push_back(y_vals[i].real());
        imag.push_back(y_vals[i].imag());
    }

    plt::figure();
    plt::scatter(indexes, real, 20.0);
    plt::scatter(indexes, imag, 20.0);
    plt::title(title);
    plt::xlabel(x_lable);
    plt::ylabel(y_lable);
    plt::grid(true);
    if(last){
        plt::show();
    } else {
        plt::show(false);
    }
}