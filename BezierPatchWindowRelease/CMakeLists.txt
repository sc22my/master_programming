cmake_minimum_required(VERSION 3.16)

# The QT application for the coursework.

project(BezierPatchWindowRelease LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-fopenmp -Wall")

# Find openmp
find_package(OpenMP REQUIRED)

# QT stuff
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
qt_standard_project_setup()

qt_add_executable(BezierPatchWindowRelease ${SOURCES} ${HEADERS})
target_include_directories(BezierPatchWindowRelease PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(
    BezierPatchWindowRelease PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    GL
    GLU
    OpenMP::OpenMP_CXX
    cwmathlib
)

# Set where the exe should be put
install(
    TARGETS BezierPatchWindowRelease
    RUNTIME DESTINATION bin
)
