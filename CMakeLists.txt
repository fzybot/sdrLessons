cmake_minimum_required(VERSION 3.16)
project(PlutoSDR CXX)

set(CMAKE_BUILD_TYPE Debug)

# --> add Python libs
find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
find_package(PythonLibs 3.0 REQUIRED)
include_directories(${PYTHON3_INCLUDE_DIRS} ${NumPy_INCLUDE_DIRS})

    # fetch matplotlib repository
include(FetchContent)
FetchContent_Declare(
    matplotlib
    GIT_REPOSITORY https://github.com/lava/matplotlib-cpp.git
    GIT_TAG        f23347fca25219d1c42cbb91608b5556814bf572
)
FetchContent_MakeAvailable(matplotlib)
include_directories(SYSTEM ${matplotlib_SOURCE_DIR})
# --> end Python libs

# RF frontend driver
find_package(SoapySDR REQUIRED)

include_directories("include/")

set(MAIN_SOURCE_FILES
    main.cpp
    src/pluto_sdr_lib.cpp
    src/plots.cpp
)

# TESTS
add_subdirectory(./tests)

# Добавляем исполняемый файл
add_executable(main ${MAIN_SOURCE_FILES})

target_link_libraries(main 
        ${SoapySDR_LIBRARIES}     
        ${PYTHON_LIBRARIES}
        Python3::NumPy
    )


