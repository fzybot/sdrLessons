cmake_minimum_required(VERSION 3.16)
project(PlutoSDR CXX)

set(CMAKE_BUILD_TYPE Debug)

# set(PYTHON_HEADERS /usr/include/python3.12) # path to header
# set(PYTHON_STATIC  /usr/lib64             ) # path to where windows stores dlls
# add_library(python_embeded SHARED IMPORTED GLOBAL)
# set_target_properties(python_embeded PROPERTIES
#     IMPORTED_LOCATION ${PYTHON_STATIC}/libpython3.9.so
#     INTERFACE_INCLUDE_DIRECTORIES ${PYTHON_HEADERS}
#     LINKER_LANGUAGE C
#     )
find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
find_package(PythonLibs 3.0 REQUIRED)
include_directories(${PYTHON3_INCLUDE_DIRS} ${NumPy_INCLUDE_DIRS})

# populate matplotlib repository
include(FetchContent)
FetchContent_Declare(
    matplotlib
    GIT_REPOSITORY https://github.com/lava/matplotlib-cpp.git
    GIT_TAG        f23347fca25219d1c42cbb91608b5556814bf572
)
# FetchContent_GetProperties(matplotlib)
# if(NOT matplotlib_POPULATED)
#     FetchContent_Populate(matplotlib)
# endif()
FetchContent_MakeAvailable(matplotlib)

include_directories(SYSTEM ${matplotlib_SOURCE_DIR})

# RF frontend driver
find_package(SoapySDR REQUIRED)

include_directories("include/")

set(MAIN_SOURCE_FILES
    main.cpp
    src/pluto_sdr_lib.cpp
    src/plots.cpp
)

# Добавляем исполняемый файл
add_executable(main ${MAIN_SOURCE_FILES})

target_link_libraries(main 
        ${SoapySDR_LIBRARIES}     
        ${PYTHON_LIBRARIES}
        Python3::NumPy
    )
