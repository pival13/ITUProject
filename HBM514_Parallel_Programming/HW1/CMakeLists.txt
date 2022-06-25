cmake_minimum_required(VERSION 3.3.0)

set(PROJECT MatrixVectorMultiplication)
project(${PROJECT})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)

add_compile_definitions($<$<BOOL:${MATRIX_SIZE}>:MAT_SIZE=${MATRIX_SIZE}>)

add_executable(${PROJECT}
    matrixVector.cpp
)
target_include_directories(${PROJECT} PUBLIC
    ${MPI_CXX_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT}
    ${MPI_CXX_LIBRARIES}
)

add_custom_command(
    TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PROJECT}>" "${PROJECT_SOURCE_DIR}"
)

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