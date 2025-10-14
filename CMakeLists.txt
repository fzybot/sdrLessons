cmake_minimum_required(VERSION 3.16)
project(PlutoSDR C)

set(CMAKE_BUILD_TYPE Debug)

# RF frontend driver
find_package(SoapySDR REQUIRED)

include_directories("include/")

set(MAIN_SOURCE_FILES
    main.c
    src/pluto_sdr_lib.c
)

# Добавляем исполняемый файл
add_executable(main ${MAIN_SOURCE_FILES})

target_link_libraries(main ${SoapySDR_LIBRARIES})
