cmake_minimum_required(VERSION 3.20)
project(Aetherwave VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apple Silicon specific settings
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif()

# Find openFrameworks
set(OF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/openFrameworks" CACHE PATH "The root directory of openFrameworks")

# Check if openFrameworks exists
if(NOT EXISTS ${OF_ROOT})
    message(WARNING "openFrameworks not found at ${OF_ROOT}. Please install openFrameworks or set OF_ROOT variable.")
endif()

# Include openFrameworks
if(EXISTS ${OF_ROOT})
    include(${OF_ROOT}/libs/openFrameworksCompiled/project/makefileCommon/config.make.cmake)
endif()

# Source files
file(GLOB_RECURSE SOURCES
    "src/cpp/*.cpp"
    "src/cpp/*.h"
    "src/shared/*.cpp"
    "src/shared/*.h"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src/cpp
    src/shared
    config
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# Link openFrameworks if available
if(EXISTS ${OF_ROOT})
    target_link_libraries(${PROJECT_NAME} PRIVATE openFrameworks)
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy config files to build directory
configure_file(${CMAKE_SOURCE_DIR}/config/config.yaml ${CMAKE_BINARY_DIR}/bin/config.yaml COPYONLY)
