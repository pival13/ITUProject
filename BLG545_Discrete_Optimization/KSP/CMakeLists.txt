cmake_minimum_required(VERSION 3.0.0)

set(PROJECT KSP)
project(${PROJECT})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ($<NOT:$<CONFIG:Debug>>)
    if (WIN32)
        add_definitions("/O2")
    else()
        add_definitions("-O2")
    endif()
endif()

include_directories(${PROJECT_SOURCE_DIR})

add_executable(${PROJECT}
    main.cpp

    Simplex.cpp
    BranchBounds.cpp

    Solver.h
    Simplex.h
    BranchBounds.h
)

set_target_properties(
    ${PROJECT}
PROPERTIES
    DEBUG_POSTFIX "_d"
)
add_custom_command(
    TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PROJECT}>" "${PROJECT_SOURCE_DIR}"
)