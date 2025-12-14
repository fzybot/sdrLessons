#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include "plots.h" 

namespace plt = matplotlibcpp;

void plot_vector(const std::vector<int> &y_vals,    std::string title = "", 
                                                    std::string x_lable = "",
                                                    std::string y_lable = "")
{
    int N = y_vals.size();
    std::vector<int> indexes(N);
    std::iota(std::begin(indexes), std::end(indexes), 0);

    plt::scatter(indexes, y_vals, 20.0);
    plt::title(title);
    plt::xlabel(x_lable);
    plt::ylabel(y_lable);
    plt::grid(true);
    plt::show();
}
