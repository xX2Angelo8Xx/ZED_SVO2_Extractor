cmake_minimum_required(VERSION 3.20)
project(ZED_SVO2_Extractor VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =============================================================================
# ZED SDK Configuration
# =============================================================================
if(WIN32)
    # Check environment variable first
    if(DEFINED ENV{ZED_SDK_ROOT_DIR})
        set(ZED_SDK_ROOT_DIR $ENV{ZED_SDK_ROOT_DIR})
    else()
        # Default installation path
        set(ZED_SDK_ROOT_DIR "C:/Program Files (x86)/ZED SDK")
    endif()
    
    if(NOT EXISTS ${ZED_SDK_ROOT_DIR})
        message(FATAL_ERROR "ZED SDK not found at: ${ZED_SDK_ROOT_DIR}")
    endif()
    
    message(STATUS "ZED SDK found at: ${ZED_SDK_ROOT_DIR}")
    
    # ZED SDK includes and libraries
    set(ZED_INCLUDE_DIRS "${ZED_SDK_ROOT_DIR}/include")
    set(ZED_LIBRARY_DIR "${ZED_SDK_ROOT_DIR}/lib")
    set(ZED_LIBRARIES "${ZED_LIBRARY_DIR}/sl_zed64.lib")
    
    # Bundled OpenCV (Version 3.1.0)
    set(OpenCV_DIR "${ZED_SDK_ROOT_DIR}/opencv")
    set(OpenCV_INCLUDE_DIRS "${OpenCV_DIR}/include")
    set(OpenCV_LIBRARY_DIR "${OpenCV_DIR}/x64/vc14/lib")
    
    # OpenCV libraries we need
    set(OpenCV_LIBS
        "${OpenCV_LIBRARY_DIR}/opencv_core310.lib"
        "${OpenCV_LIBRARY_DIR}/opencv_highgui310.lib"
        "${OpenCV_LIBRARY_DIR}/opencv_imgproc310.lib"
        "${OpenCV_LIBRARY_DIR}/opencv_imgcodecs310.lib"
    )
    
    # Add DLL directories to PATH for runtime
    set(ZED_DLL_DIR "${ZED_SDK_ROOT_DIR}/bin")
    set(OpenCV_DLL_DIR "${OpenCV_DIR}/x64/vc14/bin")
endif()

# =============================================================================
# CUDA Configuration (Optional for now, required for depth processing later)
# =============================================================================
find_package(CUDAToolkit QUIET)
if(CUDAToolkit_FOUND)
    message(STATUS "CUDA Toolkit found: ${CUDAToolkit_VERSION}")
    set(CUDA_AVAILABLE ON)
else()
    message(WARNING "CUDA Toolkit not found - depth processing features will be limited")
    set(CUDA_AVAILABLE OFF)
endif()

# =============================================================================
# Common Library (Shared utilities and metadata system)
# =============================================================================
add_subdirectory(common)

# =============================================================================
# Applications (Will be added as we develop them)
# =============================================================================
# Phase 2-3: Frame Extractor
# add_subdirectory(apps/frame_extractor)

# Phase 4-5: Video Extractor  
# add_subdirectory(apps/video_extractor)

# Phase 6-9: Depth Analyzer
# add_subdirectory(apps/depth_analyzer)

# =============================================================================
# Installation Rules
# =============================================================================
# Copy runtime dependencies (DLLs) to output directory
if(WIN32)
    # Copy ZED SDK DLLs
    install(DIRECTORY "${ZED_DLL_DIR}/"
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            FILES_MATCHING PATTERN "*.dll")
    
    # Copy OpenCV DLLs
    install(DIRECTORY "${OpenCV_DLL_DIR}/"
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            FILES_MATCHING PATTERN "*.dll")
endif()

# =============================================================================
# Build Summary
# =============================================================================
message(STATUS "")
message(STATUS "=== ZED SVO2 Extractor Build Configuration ===")
message(STATUS "Project Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  ZED SDK: ${ZED_SDK_ROOT_DIR}")
message(STATUS "  OpenCV: ${OpenCV_DIR} (bundled with ZED)")
if(CUDA_AVAILABLE)
    message(STATUS "  CUDA: ${CUDAToolkit_VERSION} ✓")
else()
    message(STATUS "  CUDA: Not found (optional)")
endif()
message(STATUS "")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "==============================================")
message(STATUS "")
