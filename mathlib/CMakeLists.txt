cmake_minimum_required(VERSION 3.16)

# Mathlib dynamic library

project(cwmathlib LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-fopenmp -Wall")

find_package(OpenMP REQUIRED)

add_library(cwmathlib STATIC ${SOURCES} ${HEADERS})
target_link_libraries(cwmathlib PRIVATE OpenMP::OpenMP_CXX)
target_include_directories(cwmathlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
