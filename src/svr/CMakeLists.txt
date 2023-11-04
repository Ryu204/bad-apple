# In this file folder run: 
# mkdir build
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# cmake --build build # append "--config Release" if you're using windows and msbuild

cmake_minimum_required(VERSION 3.20)

project(bad-apple-server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

SET(${PROJECT_NAME}_SOURCE 
    "src/main.cpp"
    "src/port.hpp"
    "src/port.cpp"
)

find_package(cserialport REQUIRED)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCE})
target_link_libraries(${PROJECT_NAME} PUBLIC cserialport)
