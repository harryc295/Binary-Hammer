cmake_minimum_required(VERSION 3.28)
project(binaryhammer)

set(GLFW_ROOT "C:/glfw3")
set(GLFW_LIBRARY "${GLFW_ROOT}/lib-vc2022/glfw3.lib")
set(GLFW_INCLUDE_DIR "${GLFW_ROOT}/include")

if(NOT EXISTS ${GLFW_LIBRARY})
    message(FATAL_ERROR "GLFW library not found at ${GLFW_LIBRARY}")
endif()

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${GLFW_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
)

include_directories("incl/")

find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "src/*.h" "src/*.c" "incl/*.h" "incl/*.cpp")
add_executable(binaryhammer ${SOURCES})

target_link_libraries(binaryhammer 
    glfw
    OpenGL::GL
)

if(WIN32)
    target_compile_definitions(binaryhammer PRIVATE GLFW_STATIC)
endif()