#include <vector>
#include <complex>
#include <string>
#include <numeric>

#include "matplotlibcpp.h"


void plot_vector(const std::vector<int> &y_vals,    std::string title, 
                                                    std::string x_lable,
                                                    std::string y_lable, bool last);


void plot_vector(const std::vector<std::complex<double>> &y_vals,   std::string title, 
                                                                    std::string x_lable,
                                                                    std::string y_lable, bool last);
