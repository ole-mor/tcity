cmake_minimum_required(VERSION 3.10)
project(tcity)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLAD
add_library(glad src/glad/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# Create the executable
add_executable(tcity src/main.cpp src/Shader.cpp src/RenderObject.cpp)
target_link_libraries(tcity glfw glad ${CMAKE_DL_LIBS})

# Include directories for GLFW and GLAD
target_include_directories(tcity PRIVATE external/glad/include external/glfw/include include)
