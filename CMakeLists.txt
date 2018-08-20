project(OpenGL LANGUAGES CXX)
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(LINK_LIBS pthread X11 Xrandr Xxf86vm Xi GLU GL GLEW glfw)

add_executable(window
    window/src/main.cpp
    )

target_link_libraries(window
    ${LINK_LIBS}
    )

add_executable(cube
    cube/src/main.cpp
    )

target_link_libraries(cube
    ${LINK_LIBS}
    )

add_executable(triangle
    triangle/src/main.cpp
    )

target_link_libraries(triangle
    ${LINK_LIBS}
    )
