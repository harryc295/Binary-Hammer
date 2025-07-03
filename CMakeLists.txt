cmake_minimum_required(VERSION 3.28)
project (binaryhammer)

file (GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "src/*.h" "src/*.c")

add_executable(binaryhammer ${SOURCES})
