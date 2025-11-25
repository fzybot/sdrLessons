// Author: https://github.com/Nicoskin/OFDM_transceiver/blob/main/other/plots.h

#include <vector>
#include <complex>
#include <string>
#include <numeric>

#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;
using cd = std::complex<double>;

// Функция для отображения графика с комплексными числами
void cool_plot(const std::vector<cd>& data, 
               std::string title = "", 
               std::string vid = "-", 
               bool show_plot = false);

// Функция для отображения графика с одним вектором чисел (y)
template <typename T>
void cool_plot(const std::vector<T>& input, 
               std::string title = "", 
               std::string vid = "-", 
               bool show_plot = false);


// TODO: fix
// void cool_scatter(const std::vector<cd>& x, const std::string title = "", bool show_plot = false);

// TODO: install FFT lib
// void spectrogram_plot(const std::vector<cd>& input, const std::string& title = "", bool mirror_mid = true, size_t FFT_Size = N_FFT, bool show_plot = false);

void show_plot();
