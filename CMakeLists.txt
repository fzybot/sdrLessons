# Минимальная версия Qt, которая поддерживает Qt Charts
cmake_minimum_required(VERSION 3.16)
project(PlutoSDR CXX C)

# Опция для включения или отключения установки зависимостей
option(INSTALL_DEPS "Установить зависимости" ON)

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)
find_package(Gnuradio REQUIRED)

# add_library(iiostream_helper STATIC src/iiostream-common.c)
# target_link_libraries(iiostream_helper PUBLIC iio)
# find_package(iio)

set(SOURCE_FILES
    src/main.cpp
)

# Путь до необходимых библиотек
# include_directories(${PATH}/libiio)
# link_directories(${PATH}/libiio)


# Добавляем исполняемый файл
add_executable(main ${SOURCE_FILES})

# Линковка с библиотеками (Qt, iio)
target_link_libraries(main Qt5::Core Qt5::Widgets Qt5::Charts iio)
