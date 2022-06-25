cmake_minimum_required(VERSION 2.8.0)

project(LinearEquation)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS --std=c++11)

find_package(MPI REQUIRED)

# add_compile_definitions($<$<BOOL:${MATRIX_SIZE}>:MAT_SIZE=${MATRIX_SIZE}>)
# add_compile_definitions($<$<BOOL:${RANDOM_SEED}>:RAND_SEED=${RANDOM_SEED}>)
# add_compile_definitions($<$<BOOL:${ITERATION}>:MAX_ITER=${ITERATION}>)
include_directories(${MPI_CXX_INCLUDE_DIRS})
link_libraries(${MPI_CXX_LIBRARIES})

add_executable(LU LinearEquation.cpp)
target_compile_definitions(LU PUBLIC DIRECT)

add_executable(Jacobi LinearEquation.cpp)  
target_compile_definitions(Jacobi PUBLIC JACOBI)

add_executable(GaussSeidel LinearEquation.cpp)
target_compile_definitions(GaussSeidel PUBLIC GS)

add_custom_command(
    TARGET LU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:LU>" "${PROJECT_SOURCE_DIR}")
add_custom_command(
    TARGET Jacobi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Jacobi>" "${PROJECT_SOURCE_DIR}")
add_custom_command(
    TARGET GaussSeidel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GaussSeidel>" "${PROJECT_SOURCE_DIR}")

# if (WIN32)
#     if ($<NOT:$<CONFIG:Debug>>)
#         add_definitions("/O2")
#     endif()
# else()
#     add_compile_options(-msse4.2)
#     if ($<CONFIG:Debug>)
#         add_definitions("-g")
#     else()
#         add_definitions("-O2")
#     endif()
# endif()