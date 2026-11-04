cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME ece2049-starter-code)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME} C CXX ASM)
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# CUSTOM: Enable C++ linkage support
set_target_properties(
    ${CMAKE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# CUSTOM: C++ compile options setup
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-use-cxa-atexit>
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here

    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_GFX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_SPITFT.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_ST77xx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_ST7735.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_ST7735_API.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Print.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_seesaw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_TFTShield18.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/graphics/Adafruit_TFTShield18_API.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/dwt_module.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/buzzer.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/graphics
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
