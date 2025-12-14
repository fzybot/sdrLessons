#include "bit_generation.h"
#include "matplotlibcpp.h"
#include "plots.h"
namespace plt = matplotlibcpp;


int main(){

    std::vector<int> test_vec = generate_bit_array(10);
    plot_vector(test_vec, "Генерируем случайный битовый набор", "Индекс массива", "Значение Бита");

    return 0;
}