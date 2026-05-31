cmake_minimum_required(VERSION 3.15)
project(NeonAlleyway CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
find_package(OpenGL REQUIRED)

# SWAPPED TO TEST FILE HERE
add_executable(NeonAlleyway 
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Pavement.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c"
)

target_link_libraries(NeonAlleyway 
    glfw3 
    ${OPENGL_LIBRARIES}
    gdi32 
    user32 
    kernel32
)