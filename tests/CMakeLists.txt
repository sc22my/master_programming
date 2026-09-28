cmake_minimum_required(VERSION 3.16)

project(tests LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

add_executable(tests ${SOURCES} ${HEADERS})
target_link_libraries(tests PRIVATE cwmathlib)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set where the exe should be put
install(
    TARGETS tests
    RUNTIME DESTINATION bin
)