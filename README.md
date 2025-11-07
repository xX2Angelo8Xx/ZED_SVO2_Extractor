# ZED SVO2 Extraction Tools Suite

[![Version](https://img.shields.io/badge/version-0.1.0--beta-blue.svg)](https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-orange.svg)](https://en.cppreference.com/w/cpp/17)
[![Status](https://img.shields.io/badge/status-Phase%202%20Complete-green.svg)]()

**Professional C++ applications for processing Stereolabs ZED 2i camera SVO2 files:**

1. **🖥️ GUI Extractor** - Modern GUI with real-time progress tracking
2. **🎬 Video Extractor CLI** - Export high-quality AVI videos (MJPEG)
3. **📸 Frame Extractor CLI** - Extract frames for YOLO model training

**Current Phase:** Phase 2B Complete - GUI Application with Threading  
**Project Start:** November 7, 2025

---

## 📚 Documentation

**Essential Reading:**
- **[📋 Project Roadmap](docs/ROADMAP.md)** - Development phases, milestones, and timeline
- **[🎓 C++ Learning Guide](docs/CPP_LEARNING_GUIDE.md)** - Learn C++ through this project
- **[⚙️ Dependency Setup](docs/DEPENDENCY_SETUP.md)** - Installation guide for all dependencies
- **[🔄 Git Workflow](docs/GIT_WORKFLOW.md)** - Version control best practices
- **[📝 Code Style Guide](docs/CODE_STYLE_GUIDE.md)** - Coding standards and conventions
- **[🔧 Implementation Details](docs/IMPLEMENTATION.md)** - Technical specifications

---

## 🎯 Project Goals

### GUI Extractor
Modern Dear ImGui-based interface with real-time extraction monitoring:
- **Video Extraction**: Left, Right, Side-by-Side, or Both cameras separately
- **Frame Extraction**: 1 FPS to 100 FPS with Left/Right camera selection
- **Real-time Progress**: Live progress bars with frame counts and status messages
- **Cancellation Support**: Stop extraction at any time
- **Flight Folder Detection**: Automatic flight name extraction from paths
- **Quality Control**: 0-100% video quality slider
- **Intelligent Output**: Automatic folder structure with extraction numbering

### Video Extractor CLI
Command-line video extraction with full control:
- AVI format with MJPEG codec (universally compatible)
- Configurable FPS (1-100), camera mode, and quality (0-100%)
- Automatic fallback to source FPS if requested exceeds source
- Flight folder detection and intelligent output paths

### Frame Extractor CLI
Extract frames for machine learning (YOLO training):
- Configurable FPS (1-100) with automatic validation
- PNG format (lossless)
- Global frame numbering across sessions
- Left or Right camera selection
- YOLO-compatible folder structure

---

## 🚀 Current Status

### ✅ Completed (Phases 0-2B)
- [x] **Phase 0**: Project structure, metadata system, documentation
- [x] **Phase 1**: Core infrastructure (error handling, file utilities, SVO handlers)
- [x] **Phase 2M1**: Metadata system with JSON export
- [x] **Phase 2M2.1**: Frame Extractor CLI (tested: 716 frames extracted)
- [x] **Phase 2A**: Video Extractor CLI with codec support
- [x] **Phase 2B**: GUI Application with Dear ImGui
- [x] **Threading Architecture**: Extraction engine library with progress callbacks
- [x] **OpenCV 4.10.0**: Upgraded from 3.1.0 for better codec support
- [x] **Real-time Progress**: Live progress bars and cancellation
- [x] **Flight Folder Detection**: Automatic parsing of flight_YYYYMMDD_HHMMSS

### 🎯 Next Up (Phase 3)
- [ ] **Phase 3A**: Depth Map Extraction
- [ ] **Phase 3B**: Neural Depth Heatmap Visualization
- [ ] **Phase 3C**: Object Detection Integration

**See [ROADMAP.md](docs/ROADMAP.md) for complete development plan.**

---

## 📋 Prerequisites

Before building and running this project, ensure you have the following installed:

### Required Software

1. **ZED SDK 4.x**
   - Download from: [Stereolabs Download Center](https://www.stereolabs.com/developers/release/)
   - Supported OS: Windows 10/11, Ubuntu 18.04+
   - Includes CUDA support

2. **CUDA Toolkit**
   - Version compatible with ZED SDK (usually 11.x or 12.x)
   - Download from: [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)

3. **OpenCV 4.10.0+**
   - **Required**: Version 4.10.0 or later (includes full FFmpeg codec support)
   - Download from: [OpenCV Releases](https://opencv.org/releases/)
   - Windows: Install to `C:\opencv` for automatic detection
   - The project uses external OpenCV, not ZED SDK's bundled 3.1.0

4. **CMake**
   - Version 3.15 or later
   - Download from: [CMake Downloads](https://cmake.org/download/)

5. **C++ Compiler**
   - **Windows**: Visual Studio 2019/2022 (with C++ development tools)
   - **Linux**: GCC 9+ or Clang 10+

6. **Dear ImGui** (included as submodule)
   - Automatically fetched by CMake via FetchContent
   - GLFW backend for windowing

## 🚀 Getting Started

### Clone the Repository

```bash
git clone https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor.git
cd ZED_SVO2_Extractor
```

### Build Instructions

#### Windows (Visual Studio)

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake (auto-detects OpenCV 4.10.0 at C:\opencv)
cmake .. -G "Visual Studio 17 2022" -A x64

# Build all targets
cmake --build . --config Release

# Executables will be in: build\bin\Release\
# - gui_extractor.exe
# - frame_extractor_cli.exe
# - video_extractor_cli.exe
```

#### Linux

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project (use all CPU cores)
make -j$(nproc)

# Executables will be in: build/bin/
```

### OpenCV Configuration

The project automatically detects OpenCV 4.10.0 at `C:\opencv\build` (Windows). If you have OpenCV installed elsewhere:

```powershell
# Set OpenCV_DIR before running CMake
cmake .. -DOpenCV_DIR="C:/path/to/opencv/build"
```

Or disable external OpenCV to use ZED SDK's bundled version (not recommended):

```powershell
cmake .. -DUSE_EXTERNAL_OPENCV=OFF
```

## 📖 Usage

### GUI Application (Recommended)

The modern GUI provides the easiest way to extract videos and frames:

```powershell
# Windows
cd build\bin\Release
.\gui_extractor.exe
```

**Features:**
- Browse for SVO files and output directories
- Configure extraction settings with sliders and dropdowns
- Real-time progress bars with frame counts
- Cancel button to stop extraction
- Automatic flight folder detection
- Result messages with output paths

### Frame Extractor CLI

Extract frames for YOLO training:

```powershell
# Basic usage (uses defaults: left camera, 1 FPS)
.\frame_extractor_cli.exe "E:\path\to\flight_20251105_205224\video.svo2"

# Custom settings
.\frame_extractor_cli.exe ^
    "E:\path\to\video.svo2" ^
    --output "E:\custom\output\path" ^
    --fps 5 ^
    --camera right

# Show help
.\frame_extractor_cli.exe --help
```

**Options:**
- `--fps N`: Extraction rate (1-100 FPS, validates against source)
- `--camera left|right`: Camera selection
- `--output PATH`: Custom output directory

**Output Structure:**
```
<output>/Yolo_Training/Unfiltered_Images/flight_YYYYMMDD_HHMMSS/
├── L_frame_000001.png
├── L_frame_000002.png
└── ...
```

### Video Extractor CLI

Export videos from SVO files:

```powershell
# Basic usage (uses defaults: left camera, source FPS, 100% quality)
.\video_extractor_cli.exe "E:\path\to\video.svo2"

# Custom settings
.\video_extractor_cli.exe ^
    "E:\path\to\video.svo2" ^
    --output "E:\custom\output\path" ^
    --fps 30 ^
    --quality 90 ^
    --camera side_by_side

# Show help
.\video_extractor_cli.exe --help
```

**Options:**
- `--fps N`: Target FPS (1-100, auto-fallback to source if exceeded)
- `--quality N`: Video quality (0-100%)
- `--camera left|right|both_separate|side_by_side`: Camera mode
- `--output PATH`: Custom output directory

**Output Structure:**
```
<output>/Extractions/flight_YYYYMMDD_HHMMSS/extraction_NNN/
├── video_left.avi (MJPEG codec)
├── video_right.avi (if applicable)
├── metadata.json
└── extraction_log.txt
```

### Configuration

Default paths are configured for:
- **Input**: `E:\Turbulence Solutions\AeroLock\ZED_Recordings`
- **Output**: `E:\Turbulence Solutions\AeroLock\ZED_Recordings_Output`

You can override these with command-line arguments or modify the defaults in the code.

---

# Generate Makefiles
cmake ..

# Build the project (use all CPU cores)
make -j$(nproc)

# The executable will be in: build/ZED_SVO2_Extractor
```

## 📖 Usage

### Basic Usage

The application uses default input/output directories configured in the source code:

- **Input Directory**: `E:\Turbulence Solutions\AeroLock\ZED_Recordings`
- **Output Directory**: `E:\Turbulence Solutions\AeroLock\ZED_Recordings_Output`

Simply run the executable:

```powershell
# Windows
.\ZED_SVO2_Extractor.exe

# Linux
./ZED_SVO2_Extractor
```

### Custom Directories

You can override the default directories via command-line arguments:

```powershell
# Specify input directory only
.\ZED_SVO2_Extractor.exe "C:\My\SVO\Files"

# Specify both input and output directories
.\ZED_SVO2_Extractor.exe "C:\My\SVO\Files" "D:\Output\Frames"
```

### Configuration Options

Edit the `ExtractionConfig` structure in `src/main.cpp` to customize:

```cpp
struct ExtractionConfig {
    std::string inputFolder;        // Input directory path
    std::string outputFolder;       // Output directory path
    std::string imageFormat;        // "png", "jpg", "bmp", etc.
    bool extractDepth;              // Extract depth maps (true/false)
    bool extractLeftImage;          // Extract left camera (true/false)
    bool extractRightImage;         // Extract right camera (true/false)
    int frameSkip;                  // Extract every Nth frame (1 = all)
};
```

## 📁 Output Structure

The tool creates an organized directory structure for each SVO2 file:

```
ZED_Recordings_Output/
├── recording_2025-11-07_10-30-00/
│   ├── left/
│   │   ├── frame_000001.png
│   │   ├── frame_000002.png
│   │   └── ...
│   ├── right/
│   │   ├── frame_000001.png
│   │   ├── frame_000002.png
│   │   └── ...
│   └── depth/
│       ├── frame_000001.png
│       ├── frame_000002.png
│       └── ...
├── recording_2025-11-07_14-20-15/
│   └── ...
└── ...
```

## 🔧 Troubleshooting

### Common Issues

#### OpenCV DLLs Not Found

Ensure the following DLLs are in your executable directory (`build\bin\Release\`):
- `opencv_world4100.dll`
- `opencv_videoio_ffmpeg4100_64.dll`
- `opencv_videoio_msmf4100_64.dll`

These are automatically copied during build from `C:\opencv\build\x64\vc16\bin`.

#### GUI Won't Start

If the GUI application crashes on startup:
1. Verify all OpenCV DLLs are present
2. Check that your GPU supports OpenGL 3.0+
3. Update your graphics drivers

#### Video Extraction Creates Small Files

If video files are only a few hundred bytes:
- This was a known issue with BGRA→BGR conversion (now fixed)
- Rebuild the project to get the latest fixes
- Ensure you're using OpenCV 4.10.0+

#### ZED SDK Not Found

```
CMake Error: Could not find ZED SDK
```

**Solution**: 
- Ensure ZED SDK is properly installed at `C:\Program Files (x86)\ZED SDK`
- Set `ZED_SDK_ROOT_DIR` environment variable if installed elsewhere

#### CUDA Errors During Extraction

Some SVO files may show IMU warnings or CUDA errors:
```
[WARNING] IMU data issue detected : IMU: HIGH_VIBRATION_ACC
[WARNING] Frames may be corrupted or degraded
```

This is usually not critical and extraction will continue. If extraction fails:
- Try a different SVO file to verify the tool works
- Check if the SVO file is corrupted
- Ensure your NVIDIA drivers are up to date

## 📊 Technical Details

### Architecture

- **Extraction Engine**: Shared library (`zed_common`) with progress callbacks
- **Threading Model**: GUI runs extraction in separate thread for responsive UI
- **Video Codec**: MJPEG in AVI container (universally compatible)
- **Frame Format**: PNG (lossless) with YOLO-compatible naming

### Performance

Typical processing times:
- **Frame Extraction**: ~60-100 FPS (depends on disk I/O)
- **Video Extraction**: ~Real-time (1x playback speed for MJPEG encoding)

### Output Formats

**Videos**: 
- Container: AVI
- Codec: MJPEG (Motion JPEG)
- Quality: Configurable 0-100% (default 100%)
- FPS: 1-100 with validation against source

**Frames**: 
- Format: PNG (lossless)
- Naming: `L_frame_XXXXXX.png` or `R_frame_XXXXXX.png`
- Numbering: Global counter across all sessions

## 🛠️ Development

### Project Structure

```
ZED_SVO2_Extractor/
├── CMakeLists.txt              # Root build configuration
├── README.md                   # This file
├── LICENSE                     # MIT License
├── .gitignore                  # Git ignore rules
├── common/                     # Shared library (zed_common)
│   ├── metadata.cpp/hpp        # Flight metadata and JSON export
│   ├── file_utils.cpp/hpp      # File operations and flight detection
│   ├── svo_handler.cpp/hpp     # ZED SVO file handling
│   ├── error_handler.cpp/hpp   # Error codes and logging
│   ├── output_manager.cpp/hpp  # Output path management
│   └── extraction_engine.cpp/hpp # Core extraction with progress
├── apps/                       # Application executables
│   ├── frame_extractor/        # Frame extraction CLI
│   ├── video_extractor/        # Video extraction CLI
│   └── gui_extractor/          # GUI application (Dear ImGui)
│       ├── main.cpp
│       └── gui_application.cpp/hpp
├── external/                   # Third-party libraries
│   └── imgui/                  # Dear ImGui (submodule)
├── docs/                       # Comprehensive documentation
│   ├── ROADMAP.md
│   ├── IMPLEMENTATION.md
│   ├── CPP_LEARNING_GUIDE.md
│   ├── CODE_STYLE_GUIDE.md
│   ├── DEPENDENCY_SETUP.md
│   └── GIT_WORKFLOW.md
└── build/                      # Build output (git-ignored)
```

### Building from Source

See [Build Instructions](#getting-started) above.

### Running Tests

Currently manual testing is performed. Automated testing framework planned for Phase 4.

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Angelo Amon** (xX2Angelo8Xx)
- GitHub: [@xX2Angelo8Xx](https://github.com/xX2Angelo8Xx)

## 🙏 Acknowledgments

- [Stereolabs](https://www.stereolabs.com/) for the ZED SDK and camera hardware
- [OpenCV](https://opencv.org/) community for image/video processing
- [Dear ImGui](https://github.com/ocornut/imgui) for the excellent GUI framework
- [NVIDIA](https://developer.nvidia.com/) for CUDA support

## 📞 Support

If you encounter any issues:

1. Check the [Troubleshooting](#-troubleshooting) section
2. Review [ROADMAP.md](docs/ROADMAP.md) for known limitations
3. Check [IMPLEMENTATION.md](docs/IMPLEMENTATION.md) for technical details
4. Open an issue on GitHub with logs and error messages

## 🗺️ Roadmap

See [ROADMAP.md](docs/ROADMAP.md) for the complete development plan.

**Next Phases:**
- **Phase 3**: Depth map extraction and visualization
- **Phase 4**: Testing and optimization
- **Phase 5**: Advanced features (H.264 via FFmpeg, batch processing)

## 📅 Version History

### v0.1.0-beta (2025-11-07)
- ✅ Frame Extractor CLI with configurable FPS
- ✅ Video Extractor CLI with MJPEG codec
- ✅ GUI Application with Dear ImGui
- ✅ Real-time progress tracking and cancellation
- ✅ OpenCV 4.10.0 integration
- ✅ Flight folder detection
- ✅ Metadata system with JSON export
- ✅ Comprehensive documentation

---

**Made with ❤️ for drone flight analysis and YOLO training**

