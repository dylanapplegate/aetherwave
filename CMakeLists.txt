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

# Find SDL2 (much lighter than openFrameworks)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Source files (using real SDL2 visual app)
set(SOURCES
    "src/cpp/visual_sdl.cpp"
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

# Link SDL2 libraries
target_include_directories(${PROJECT_NAME} PRIVATE
    src/cpp
    src/shared
    config
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# Add SDL2 compile flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})

# Add filesystem library for C++17
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy config files to build directory
configure_file(${CMAKE_SOURCE_DIR}/config/config.yaml ${CMAKE_BINARY_DIR}/bin/config.yaml COPYONLY)

# Asset management: sync source assets to build directory
set(SOURCE_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
set(BUILD_ASSETS_DIR "${CMAKE_BINARY_DIR}/bin/assets")

# Create assets directory structure in build
file(MAKE_DIRECTORY "${BUILD_ASSETS_DIR}/images")

# Copy README files (always tracked)
configure_file("${SOURCE_ASSETS_DIR}/README.md" "${BUILD_ASSETS_DIR}/README.md" COPYONLY)
configure_file("${SOURCE_ASSETS_DIR}/images/README.md" "${BUILD_ASSETS_DIR}/images/README.md" COPYONLY)

# Function to copy assets during build
function(sync_assets)
    # Get list of image files from source assets
    file(GLOB_RECURSE ASSET_FILES
        "${SOURCE_ASSETS_DIR}/images/*.jpg"
        "${SOURCE_ASSETS_DIR}/images/*.jpeg"
        "${SOURCE_ASSETS_DIR}/images/*.png"
        "${SOURCE_ASSETS_DIR}/images/*.gif"
        "${SOURCE_ASSETS_DIR}/images/*.bmp"
        "${SOURCE_ASSETS_DIR}/images/*.tiff"
        "${SOURCE_ASSETS_DIR}/images/*.tif"
    )

    # Copy each asset file, preserving directory structure
    foreach(ASSET_FILE ${ASSET_FILES})
        # Get relative path from source assets directory
        file(RELATIVE_PATH REL_PATH "${SOURCE_ASSETS_DIR}" "${ASSET_FILE}")
        set(DEST_FILE "${BUILD_ASSETS_DIR}/${REL_PATH}")

        # Get directory of destination file
        get_filename_component(DEST_DIR "${DEST_FILE}" DIRECTORY)
        file(MAKE_DIRECTORY "${DEST_DIR}")

        # Copy file if source is newer than destination
        configure_file("${ASSET_FILE}" "${DEST_FILE}" COPYONLY)
    endforeach()

    list(LENGTH ASSET_FILES ASSET_COUNT)
    if(ASSET_COUNT GREATER 0)
        message(STATUS "📸 Synced ${ASSET_COUNT} asset files to build directory")
    else()
        message(STATUS "📁 No asset files found in ${SOURCE_ASSETS_DIR}/images/")
        message(STATUS "   Add images to assets/images/ for display content")
    endif()
endfunction()

# Call asset sync function
sync_assets()
