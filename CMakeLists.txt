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

# Source files (using simplified version that actually runs)
set(SOURCES
    "src/cpp/simple_app.cpp"
    "src/shared/Config.cpp"
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

# For now, build without openFrameworks to test basic compilation
# We'll add openFrameworks integration later
if(APPLE)
    # Find OpenGL and other required frameworks for basic compilation
    find_package(OpenGL REQUIRED)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenGL_LIBRARIES}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework CoreFoundation"
        "-framework AppKit"
        "-framework OpenGL"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework QuartzCore"
        "-framework Security"
        "-framework ApplicationServices"
    )
    
    # Add filesystem library for C++17
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
    endif()
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy config files to build directory
configure_file(${CMAKE_SOURCE_DIR}/config/config.yaml ${CMAKE_BINARY_DIR}/bin/config.yaml COPYONLY)
