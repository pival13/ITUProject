cmake_minimum_required(VERSION 3.9.0)

project(3DPoisson)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS --std=c++11)

find_package(MPI REQUIRED)
#find_package(OpenMP REQUIRED)

include_directories(${MPI_CXX_INCLUDE_DIRS})
link_libraries(${MPI_CXX_LIBRARIES}) #OpenMP::OpenMP_CXX)

add_executable(a 3D_poisson.cpp)

add_custom_command(
    TARGET a POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:a>" "${PROJECT_SOURCE_DIR}")