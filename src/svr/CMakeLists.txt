# In this file folder run: 
# mkdir build
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# cmake --build build # append "--config Release" if you're using windows and msbuild

cmake_minimum_required(VERSION 3.20)

project(bad-apple-server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

SET(${PROJECT_NAME}_SOURCE 
    "src/main.cpp"
    "src/port.hpp"
    "src/port.cpp"
    "src/video.hpp"
    "src/video.cpp"
    "src/ext/src/glad.c"
    "src/ext/include/glad/glad.h"
    "src/ext/include/KHR/khrplatform.h"
)

find_package(cserialport REQUIRED)
find_package(OpenCV REQUIRED core imgcodecs videoio highgui)
find_package(SFML 2.6 REQUIRED window)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCE})
target_link_libraries(${PROJECT_NAME} PUBLIC cserialport ${OpenCV_LIBS} sfml-window)
