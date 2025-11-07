# Dear ImGui Library Configuration
# This file downloads and builds Dear ImGui with GLFW and OpenGL3 backends

cmake_minimum_required(VERSION 3.15)

# Download Dear ImGui
include(FetchContent)

# GLFW for window management
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)

# Dear ImGui (no direct git, we'll download manually or use submodule)
# For now, we'll add ImGui files manually
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Dear ImGui directory")

# Download GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

# Create ImGui library
# Note: User needs to download ImGui source files to external/imgui/
# Get from: https://github.com/ocornut/imgui

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui.h")
    message(STATUS "Dear ImGui found in ${CMAKE_CURRENT_SOURCE_DIR}")
    
    add_library(imgui STATIC
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        backends/imgui_impl_glfw.cpp
        backends/imgui_impl_opengl3.cpp
    )
    
    target_include_directories(imgui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backends
    )
    
    target_link_libraries(imgui PUBLIC
        glfw
        opengl32
    )
    
    target_compile_definitions(imgui PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GLAD
    )
else()
    message(WARNING "Dear ImGui source not found. Please download from https://github.com/ocornut/imgui")
    message(WARNING "Extract to: ${CMAKE_CURRENT_SOURCE_DIR}")
    
    # Create dummy target so build doesn't fail
    add_library(imgui INTERFACE)
endif()
