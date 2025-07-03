cmake_minimum_required(VERSION 3.28)
project (binaryhammer)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

file (GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "src/*.h" "src/*.c" "incl/*.h" "incl/*.cpp")

add_executable(binaryhammer ${SOURCES})

target_link_libraries(binaryhammer glfw)
target_link_libraries(binaryhammer OpenGL::GL)
