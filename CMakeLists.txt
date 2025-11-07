# ============================================================================
# CMakeLists.txt - ZED SVO2 Frame Extraction Tool
# ============================================================================
# Author: Angelo Amon (xX2Angelo8Xx)
# Date: November 7, 2025
# Description: CMake build configuration for the ZED SVO2 extraction tool
# ============================================================================

cmake_minimum_required(VERSION 3.15)

# ============================================================================
# Project Configuration
# ============================================================================
project(ZED_SVO2_Extractor 
    VERSION 1.0.0
    DESCRIPTION "ZED Camera SVO2 Frame Extraction Tool"
    LANGUAGES CXX
)

# Set C++ standard to C++17 (required for filesystem)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Build Type Configuration
# ============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# Compiler Options
# ============================================================================
if(MSVC)
    # Windows Visual Studio compiler flags
    add_compile_options(
        /W4                 # Warning level 4
        /MP                 # Multi-processor compilation
        /bigobj             # Large object files support
    )
    # Add runtime library flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
else()
    # GCC/Clang compiler flags
    add_compile_options(
        -Wall               # Enable all warnings
        -Wextra             # Extra warnings
        -pedantic           # Strict ISO C++
    )
endif()

# ============================================================================
# Find Required Packages
# ============================================================================

# --- ZED SDK ---
# The ZED SDK must be installed on the system
# Download from: https://www.stereolabs.com/developers/release/
find_package(ZED 4 REQUIRED)

if(ZED_FOUND)
    message(STATUS "ZED SDK found: ${ZED_VERSION}")
    message(STATUS "ZED include directory: ${ZED_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "ZED SDK not found! Please install the ZED SDK from https://www.stereolabs.com/developers/release/")
endif()

# --- CUDA (Required by ZED SDK) ---
find_package(CUDA ${ZED_CUDA_VERSION} REQUIRED)

if(CUDA_FOUND)
    message(STATUS "CUDA found: ${CUDA_VERSION}")
    message(STATUS "CUDA include directory: ${CUDA_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "CUDA not found! Please install CUDA Toolkit compatible with ZED SDK")
endif()

# --- OpenCV ---
# OpenCV is used for image processing and saving
find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include directory: ${OpenCV_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCV not found! Please install OpenCV from https://opencv.org/releases/")
endif()

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ZED_INCLUDE_DIRS}
    ${CUDA_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# ============================================================================
# Link Directories
# ============================================================================
link_directories(
    ${ZED_LIBRARY_DIR}
    ${CUDA_LIBRARY_DIRS}
    ${OpenCV_LIBRARY_DIRS}
)

# ============================================================================
# Source Files
# ============================================================================
set(SOURCE_FILES
    src/main.cpp
)

# ============================================================================
# Executable Target
# ============================================================================
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# ============================================================================
# Linking Libraries
# ============================================================================
target_link_libraries(${PROJECT_NAME}
    ${ZED_LIBRARIES}
    ${CUDA_LIBRARIES}
    ${OpenCV_LIBS}
)

# ============================================================================
# Installation Rules
# ============================================================================
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT applications
)

# ============================================================================
# Display Configuration Summary
# ============================================================================
message(STATUS "")
message(STATUS "==========================================================")
message(STATUS "  ${PROJECT_NAME} v${PROJECT_VERSION} - Build Configuration")
message(STATUS "==========================================================")
message(STATUS "  C++ Standard:       ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:           ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  ZED SDK Version:    ${ZED_VERSION}")
message(STATUS "  CUDA Version:       ${CUDA_VERSION}")
message(STATUS "  OpenCV Version:     ${OpenCV_VERSION}")
message(STATUS "  Install Prefix:     ${CMAKE_INSTALL_PREFIX}")
message(STATUS "==========================================================")
message(STATUS "")

# ============================================================================
# Build Instructions
# ============================================================================
# To build this project:
# 
# Windows (Visual Studio):
# 1. mkdir build
# 2. cd build
# 3. cmake .. -G "Visual Studio 17 2022" -A x64
# 4. cmake --build . --config Release
#
# Windows (MinGW):
# 1. mkdir build
# 2. cd build
# 3. cmake .. -G "MinGW Makefiles"
# 4. cmake --build .
#
# Linux:
# 1. mkdir build
# 2. cd build
# 3. cmake ..
# 4. make -j$(nproc)
# ============================================================================
