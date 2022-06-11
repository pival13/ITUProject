cmake_minimum_required(VERSION 3.3.0)

set(PROJECT test)
project(${PROJECT})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    if ($<NOT:$<CONFIG:Debug>>)
        add_definitions("/O2")
    endif()
else()
    add_compile_options(-msse4.2)
    if ($<CONFIG:Debug>)
        add_definitions("-g")
    else()
        add_definitions("-O2")
    endif()
endif()

add_library(ExternalSkipList ExternalSkipList.c) # SkipList by Troy Deck. Serve as an initial comparison
add_library(BasicSkipList BasicSkipList.c)
add_library(SkipList SkipList.c)
add_library(SkipList2 SkipList2.c)
add_library(SkipListCache SkipListCache.c)
add_library(SkipListSIMD SkipListSIMD.c)

if (SKIPLIST)
    if (SKIPLIST MATCHES "External|Basic|Second")
        set(SKIPLIST_LIB ${SKIPLIST}SkipList)
    elseif (SKIPLIST MATCHES "Cache|SIMD")
        set(SKIPLIST_LIB SkipList${SKIPLIST})
    else()
        message(SEND_ERROR "Invalid parameter SKIPLIST (${SKIPLIST}). Select one of \"External\", \"Basic\", \"Second\", \"Cache\" or \"SIMD\"")
        set(SKIPLIST_LIB SkipList)
    endif()
else()
    set(SKIPLIST_LIB SkipList)
endif()

add_compile_definitions($<$<BOOL:${RANDOM_SEED}>:RANDOM_SEED=${RANDOM_SEED}>)

add_executable(testCorrectness testCorrectness.c)
target_link_libraries(testCorrectness ${SKIPLIST_LIB})
set_target_properties(testCorrectness PROPERTIES DEBUG_POSTFIX "_d")

add_executable(testSpeed testSpeed.c)
target_link_libraries(testSpeed ${SKIPLIST_LIB})
target_compile_definitions(testSpeed PUBLIC $<$<BOOL:${NUMBER_COUNT}>:NUMBER_COUNT=${NUMBER_COUNT}>)
set_target_properties(testSpeed PROPERTIES DEBUG_POSTFIX "_d")

#add_executable(testSpeedBasic testSpeed.c)
#target_link_libraries(testSpeedBasic BasicSkipList)
#target_compile_definitions(testSpeed PUBLIC $<$<BOOL:${NUMBER_COUNT}>:"NUMBER_COUNT=${NUMBER_COUNT}">)
#set_target_properties(testSpeedBasic PROPERTIES DEBUG_POSTFIX "_d")

add_custom_command(
    TARGET testSpeed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:testCorrectness>" "${PROJECT_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:testSpeed>" "${PROJECT_SOURCE_DIR}"
    #COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:testSpeedBasic>" "${PROJECT_SOURCE_DIR}"
)