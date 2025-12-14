#include <vector>
#include <complex>
#include <string>
#include <numeric>

#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;
using cd = std::complex<double>;

void plot_vector(const std::vector<int> &y_vals,    std::string title, 
                                                    std::string x_lable,
                                                    std::string y_lable);

