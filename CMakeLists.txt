cmake_minimum_required(VERSION 3.10)

project(calc_cosim VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-mfma -O3 -Ofast -ffast-math -fopenmp)

find_package(OpenMP REQUIRED)
find_package(gflags REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SOURCES)
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
target_link_libraries(${PROJECT_NAME} libgflags.a)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
