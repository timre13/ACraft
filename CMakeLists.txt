cmake_minimum_required(VERSION 3.10)

project(acraft VERSION 1.0)

option(USE_SANITIZERS "Link with address, leak and UB sanitizers" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

link_libraries(GLEW GL glfw)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror=return-type -Weffc++ -g3 -pthread")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang needs more flags to generate good debug info
    message("Clang detected")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-limit-debug-info")
else()
    message("Non-Clang compiler detected")
endif()

if (USE_SANITIZERS)
    message("Using sanitizers")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak,undefined")
endif()


add_executable(acraft
    src/main.cpp
    src/Logger.cpp
    src/ShaderProg.cpp
    src/Texture.cpp
    src/Camera.cpp
    src/obj.cpp
    deps/OpenSimplexNoise/OpenSimplexNoise/OpenSimplexNoise.cpp
)
