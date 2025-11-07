# Dear ImGui Setup

This directory should contain the Dear ImGui library files.

## Installation Steps

1. Download Dear ImGui from: https://github.com/ocornut/imgui
2. Extract the following files to this directory (`external/imgui/`):
   - imgui.h
   - imgui.cpp
   - imgui_demo.cpp
   - imgui_draw.cpp
   - imgui_internal.h
   - imgui_tables.cpp
   - imgui_widgets.cpp
   - imconfig.h
   - imstb_rectpack.h
   - imstb_textedit.h
   - imstb_truetype.h

3. Extract the `backends` folder with:
   - backends/imgui_impl_glfw.h
   - backends/imgui_impl_glfw.cpp
   - backends/imgui_impl_opengl3.h
   - backends/imgui_impl_opengl3.cpp
   - backends/imgui_impl_opengl3_loader.h

## Quick Download (Latest Version)

```bash
# Using Git (recommended)
cd external
git clone https://github.com/ocornut/imgui.git imgui_temp
cp -r imgui_temp/* imgui/
rm -rf imgui_temp

# Or download as ZIP from GitHub and extract
```

## Version

Tested with Dear ImGui v1.89+  
GLFW is automatically downloaded via CMake FetchContent.

## OpenGL

OpenGL32 is used (Windows system library).  
For Linux/Mac, may need to install OpenGL development packages.
