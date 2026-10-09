# run using xwayland
export QT_QPA_PLATFORM=xcb

# Run cmake
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug

# compile the project
cmake --build build

# Place output in the current directory
cmake --install build --prefix "."
