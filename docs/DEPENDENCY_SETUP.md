# Dependency Setup Guide

**Platform:** Windows 10/11 (Primary)  
**Compiler:** Visual Studio 2019/2022  
**Last Updated:** November 7, 2025

---

## 📋 Required Dependencies

| Dependency | Version | Purpose | Installation Time |
|------------|---------|---------|-------------------|
| Visual Studio | 2019/2022 | C++ Compiler | ~30 min |
| CMake | 3.15+ | Build System | ~5 min |
| ZED SDK | 4.x | Camera & SVO2 | ~20 min |
| CUDA Toolkit | 11/12 | GPU Processing | ~30 min |
| OpenCV | 4.x | Image Processing | ~15 min |
| Git | Latest | Version Control | ~5 min |

**Total Estimated Time:** 2-3 hours

---

## 🚀 Installation Steps

### 1. Visual Studio (Required)

**Why:** C++ compiler and development tools

**Steps:**
1. Download: https://visualstudio.microsoft.com/downloads/
2. Choose "Community" (free) edition
3. Run installer
4. Select workload: **"Desktop development with C++"**
5. Additional components (check these):
   - MSVC v142/v143 compiler
   - Windows 10/11 SDK
   - CMake tools for Windows
6. Install (requires ~10GB disk space)

**Verification:**
```powershell
# Open "Developer PowerShell for VS 2022"
cl
# Should show compiler version
```

---

### 2. Git (Required)

**Why:** Version control and repository management

**Steps:**
1. Download: https://git-scm.com/download/win
2. Run installer
3. Use default settings (Git Bash + Git from command line)
4. Complete installation

**Verification:**
```powershell
git --version
# Should show: git version 2.x.x
```

**Configuration:**
```powershell
git config --global user.name "Angelo Amon"
git config --global user.email "angelomichaelamon2001@gmail.com"
```

---

### 3. CMake (Required)

**Why:** Cross-platform build system

**Method 1: Via Visual Studio Installer**
- Already included if you selected "CMake tools for Windows"

**Method 2: Standalone Installation**
1. Download: https://cmake.org/download/
2. Choose "Windows x64 Installer"
3. Run installer
4. **Important:** Check "Add CMake to system PATH"
5. Complete installation

**Verification:**
```powershell
cmake --version
# Should show: cmake version 3.x.x
```

---

### 4. CUDA Toolkit (Required)

**Why:** Required by ZED SDK for GPU acceleration

**Prerequisites:**
- NVIDIA GPU (GTX 1000 series or newer recommended)
- Latest NVIDIA drivers

**Steps:**
1. Update GPU drivers: https://www.nvidia.com/Download/index.aspx
2. Download CUDA: https://developer.nvidia.com/cuda-downloads
3. Choose:
   - OS: Windows
   - Architecture: x86_64
   - Version: 11 or 12 (check ZED SDK compatibility)
   - Installer Type: exe (local)
4. Run installer
5. Choose "Custom" installation
6. Select:
   - CUDA Toolkit
   - CUDA Samples (optional, useful for testing)
   - CUDA Documentation (optional)
7. Complete installation (~5GB)

**Verification:**
```powershell
nvcc --version
# Should show: Cuda compilation tools, release X.X
```

**Troubleshooting:**
- If `nvcc` not found, add to PATH:
  - Default location: `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.x\bin`

---

### 5. ZED SDK (Required)

**Why:** Core library for ZED camera and SVO2 file handling

**Prerequisites:**
- CUDA Toolkit installed first
- NVIDIA GPU

**Steps:**
1. Create account: https://www.stereolabs.com/
2. Download: https://www.stereolabs.com/developers/release/
3. Choose:
   - Platform: Windows 10/11
   - Version: Latest 4.x
4. Run installer
5. Select components:
   - ZED SDK
   - Python API (optional)
   - Tools and samples (recommended for testing)
6. Complete installation (~2GB)
7. Accept license agreement

**Installation Location:**
- Default: `C:\Program Files (x86)\ZED SDK`

**Verification:**
```powershell
# Check environment variable
$env:ZED_SDK_ROOT_DIR
# Should show: C:\Program Files (x86)\ZED SDK

# Or check manually
dir "C:\Program Files (x86)\ZED SDK"
```

**Post-Installation:**
1. Restart your computer (important!)
2. Test with ZED Explorer (if you have a camera):
   - Start Menu → ZED SDK → ZED Explorer

---

### 6. OpenCV (Required)

**Why:** Image processing and saving

**Method 1: Pre-built Binaries (Recommended)**

**Steps:**
1. Download: https://opencv.org/releases/
2. Choose latest 4.x version (e.g., 4.8.0)
3. Download "Windows" package
4. Run installer (it's a self-extracting archive)
5. Extract to: `C:\opencv`
6. **Important:** Add to system environment variables:

**Setting Environment Variables:**
```powershell
# Run as Administrator
[Environment]::SetEnvironmentVariable(
    "OpenCV_DIR", 
    "C:\opencv\build", 
    "Machine"
)

# Add to PATH
$currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
$newPath = $currentPath + ";C:\opencv\build\x64\vc16\bin"
[Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
```

**Or manually:**
1. Right-click "This PC" → Properties
2. Advanced system settings
3. Environment Variables
4. System variables:
   - New: `OpenCV_DIR` = `C:\opencv\build`
   - Edit PATH: Add `C:\opencv\build\x64\vc16\bin`
5. OK, OK, OK
6. **Restart PowerShell/CMD**

**Verification:**
```powershell
# Check environment variable
$env:OpenCV_DIR
# Should show: C:\opencv\build

# Check DLLs are accessible
dir "C:\opencv\build\x64\vc16\bin\opencv_world*.dll"
```

**Method 2: Build from Source (Advanced)**
- Only if you need custom configuration
- Follow: https://docs.opencv.org/4.x/d3/d52/tutorial_windows_install.html

---

### 7. Dear ImGui (Optional - for GUI phases)

**Why:** Modern GUI framework

**Note:** Will be included as a git submodule when needed (Phase 3+)

**Setup (When Needed):**
```powershell
cd "C:\Users\Angelo Amon\OneDrive\Privat\Turbulence Solutions\Aero Lock\ZED2i\Programming\ZED_SVO2_Extractor"
git submodule add https://github.com/ocornut/imgui.git external/imgui
git submodule add https://github.com/glfw/glfw.git external/glfw
git submodule update --init --recursive
```

---

### 8. FFmpeg (Optional - for video encoding)

**Why:** Video encoding for MP4 output

**Steps:**
1. Download: https://www.gyan.dev/ffmpeg/builds/
2. Choose "ffmpeg-git-full.7z"
3. Extract to: `C:\ffmpeg`
4. Add to PATH:

```powershell
# Run as Administrator
$currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
$newPath = $currentPath + ";C:\ffmpeg\bin"
[Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
```

**Verification:**
```powershell
ffmpeg -version
# Should show: ffmpeg version N-xxxxx-gxxxxxxx
```

---

## 🏗️ Project Setup

### Clone Repository

```powershell
# Navigate to your projects folder
cd "C:\Users\Angelo Amon\OneDrive\Privat\Turbulence Solutions\Aero Lock\ZED2i\Programming"

# Clone (if not already done)
git clone https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor.git
cd ZED_SVO2_Extractor
```

### Create Build Directory

```powershell
mkdir build
cd build
```

### Configure with CMake

```powershell
# For Visual Studio 2022
cmake .. -G "Visual Studio 17 2022" -A x64

# For Visual Studio 2019
cmake .. -G "Visual Studio 16 2019" -A x64
```

**Expected Output:**
```
-- ZED SDK found: 4.x.x
-- CUDA found: 12.x
-- OpenCV found: 4.x.x
-- Configuring done
-- Generating done
```

### Build

```powershell
# Build Release version
cmake --build . --config Release

# Or Debug version
cmake --build . --config Debug
```

### Run

```powershell
# From build directory
.\Release\ZED_Frame_Extractor.exe --help
```

---

## 🔍 Troubleshooting

### Issue: "CMake not found"
**Solution:**
```powershell
# Check if installed
cmake --version

# If not, add to PATH or install via Visual Studio Installer
```

### Issue: "ZED SDK not found"
**Solution:**
```powershell
# Set environment variable
[Environment]::SetEnvironmentVariable(
    "ZED_SDK_ROOT_DIR",
    "C:\Program Files (x86)\ZED SDK",
    "Machine"
)

# Restart PowerShell
```

### Issue: "OpenCV not found"
**Solution:**
```powershell
# Check OpenCV_DIR is set
$env:OpenCV_DIR

# If not, set it (see OpenCV installation section)
```

### Issue: "CUDA not found"
**Solution:**
```powershell
# Check CUDA_PATH
$env:CUDA_PATH

# Should be: C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.x

# If not, reinstall CUDA Toolkit
```

### Issue: "Missing DLLs when running"
**Solution:**
```powershell
# Copy required DLLs to build folder
# Or ensure paths are in system PATH:
# - C:\opencv\build\x64\vc16\bin
# - C:\Program Files (x86)\ZED SDK\bin
# - C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.x\bin
```

---

## ✅ Verification Checklist

Before proceeding to development:

- [ ] Visual Studio installed with C++ support
- [ ] Git installed and configured
- [ ] CMake accessible from command line
- [ ] CUDA Toolkit installed and nvcc works
- [ ] ZED SDK installed, ZED_SDK_ROOT_DIR set
- [ ] OpenCV installed, OpenCV_DIR set
- [ ] Environment variables set correctly
- [ ] Computer restarted after installations
- [ ] Repository cloned successfully
- [ ] CMake configuration successful
- [ ] Project builds without errors

---

## 📞 Getting Help

If you encounter issues:

1. **Check official documentation:**
   - ZED SDK: https://www.stereolabs.com/docs/
   - OpenCV: https://docs.opencv.org/
   - CMake: https://cmake.org/documentation/

2. **Common issues:**
   - Restart computer after installing SDK/CUDA
   - Run PowerShell as Administrator for environment variables
   - Ensure PATH is updated correctly

3. **Developer community:**
   - Stereolabs forum: https://community.stereolabs.com/
   - OpenCV forum: https://forum.opencv.org/

---

## 🔄 Updating Dependencies

### Update ZED SDK:
1. Download latest version
2. Run installer (will update existing installation)
3. Restart computer

### Update OpenCV:
1. Download new version
2. Extract to same location or new location
3. Update OpenCV_DIR environment variable if changed

### Update Project:
```powershell
cd ZED_SVO2_Extractor
git pull origin master
cd build
cmake ..
cmake --build . --config Release
```

---

*Keep this guide handy during development!*
