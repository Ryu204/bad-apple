# To initialize CMake use: 
# cmake --preset=rel -DCMAKE_C_COMPILER=sdcc -DCMAKE_SYSTEM_NAME=Generic 

cmake_minimum_required(VERSION 3.20)

project(bad-apple C)

include(cmake/sdcc.cmake)

# Additional compile flags for each preset
if (USER_DEFINED_ADDITIONAL_FLAGS)
    message("-- Additional compile flags set: ${USER_DEFINED_ADDITIONAL_FLAGS}")
    add_compile_options(${USER_DEFINED_ADDITIONAL_FLAGS})
endif()

if (USER_DEFINED_ADDITIONAL_LINKER_FLAGS)
    message("-- Additional linking flags set: ${USER_DEFINED_ADDITIONAL_LINKER_FLAGS}")
    add_link_options(${USER_DEFINED_ADDITIONAL_LINKER_FLAGS})
endif()

# Grab all *.c file in source folder to compile
set(${PROJECT_NAME}_SOURCE 
    "src/main.c"
    "src/type.h"
    "src/monitor.h"
    "src/monitor.c"
    "src/common.h"
    "src/vga.h"
    "src/vga.c")
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCE})


# Local directories
if (USER_DEFINED_ADDITIONAL_DIR)
    message("-- Additional directories: ${USER_DEFINED_ADDITIONAL_DIR}")
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${USER_DEFINED_ADDITIONAL_DIR})
endif()