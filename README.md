# ZED SVO2 Frame Extraction Tool

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-orange.svg)](https://en.cppreference.com/w/cpp/17)

A professional C++ application for extracting frames from Stereolabs ZED camera SVO2 (Stereo Video Output) files. This tool enables batch processing of multiple SVO2 files with support for left/right camera images and depth map extraction.

## 🎯 Features

- ✅ **Batch Processing**: Process multiple SVO2 files automatically
- ✅ **Multi-View Extraction**: Extract left camera, right camera, and depth images
- ✅ **Flexible Configuration**: Customizable output format (PNG, JPEG, etc.)
- ✅ **Frame Skipping**: Extract every Nth frame to reduce output size
- ✅ **Progress Tracking**: Real-time progress display during extraction
- ✅ **Organized Output**: Automatic directory structure creation
- ✅ **Error Handling**: Robust error handling and reporting
- ✅ **Performance Metrics**: Detailed timing and statistics

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

3. **OpenCV**
   - Version 4.x or later recommended
   - Download from: [OpenCV Releases](https://opencv.org/releases/)
   - Windows: Use pre-built binaries or build from source
   - Linux: Install via package manager or build from source

4. **CMake**
   - Version 3.15 or later
   - Download from: [CMake Downloads](https://cmake.org/download/)

5. **C++ Compiler**
   - **Windows**: Visual Studio 2019/2022 (with C++ development tools)
   - **Linux**: GCC 9+ or Clang 10+

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

# Generate Visual Studio project files
cmake .. -G "Visual Studio 17 2022" -A x64

# Build the project
cmake --build . --config Release

# The executable will be in: build\Release\ZED_SVO2_Extractor.exe
```

#### Windows (MinGW)

```powershell
# Create build directory
mkdir build
cd build

# Generate Makefiles
cmake .. -G "MinGW Makefiles"

# Build the project
cmake --build .

# The executable will be in: build\ZED_SVO2_Extractor.exe
```

#### Linux

```bash
# Create build directory
mkdir build
cd build

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

#### ZED SDK Not Found

```
CMake Error: Could not find ZED SDK
```

**Solution**: 
- Ensure ZED SDK is properly installed
- Set `ZED_SDK_ROOT_DIR` environment variable to your ZED SDK installation path
- Windows: Usually `C:\Program Files (x86)\ZED SDK`
- Linux: Usually `/usr/local/zed`

#### CUDA Not Found

```
CMake Error: Could not find CUDA
```

**Solution**:
- Install CUDA Toolkit compatible with your ZED SDK version
- Add CUDA to your system PATH
- Restart your terminal/IDE after installation

#### OpenCV Not Found

```
CMake Error: Could not find OpenCV
```

**Solution**:
- Ensure OpenCV is installed
- Set `OpenCV_DIR` to your OpenCV build directory
- Windows: Usually `C:\opencv\build`
- Linux: Usually `/usr/local/lib/cmake/opencv4`

#### SVO File Won't Open

```
[ERROR] Failed to open SVO file: ERROR_CODE_INVALID_SVO_FILE
```

**Solution**:
- Verify the SVO file is not corrupted
- Ensure the SVO file was created with a compatible ZED SDK version
- Check file permissions

## 📊 Performance

Typical processing times on a modern system:
- **HD720 (1280×720)**: ~100-150 FPS extraction rate
- **HD1080 (1920×1080)**: ~60-80 FPS extraction rate
- **HD2K (2208×1242)**: ~40-50 FPS extraction rate

*Performance varies based on CPU, disk speed, and frame skip settings.*

## 🛠️ Development

### Project Structure

```
ZED_SVO2_Extractor/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── LICENSE                 # License file
├── .gitignore             # Git ignore rules
├── src/
│   └── main.cpp           # Main application source
├── include/               # Header files (for future expansion)
├── docs/                  # Additional documentation
└── build/                 # Build output (git-ignored)
```

### Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Code Style

- Use C++17 features where appropriate
- Follow consistent indentation (4 spaces)
- Add comprehensive comments for complex logic
- Use descriptive variable and function names
- Document public APIs with Doxygen-style comments

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Angelo Amon** (xX2Angelo8Xx)
- GitHub: [@xX2Angelo8Xx](https://github.com/xX2Angelo8Xx)

## 🙏 Acknowledgments

- [Stereolabs](https://www.stereolabs.com/) for the ZED SDK
- [OpenCV](https://opencv.org/) community for image processing tools
- [NVIDIA](https://developer.nvidia.com/) for CUDA support

## 📞 Support

If you encounter any issues or have questions:

1. Check the [Troubleshooting](#-troubleshooting) section
2. Review [ZED SDK Documentation](https://www.stereolabs.com/docs/)
3. Open an issue on GitHub
4. Contact: [Your Contact Information]

## 🗺️ Roadmap

Future enhancements planned:

- [ ] GUI interface for easier configuration
- [ ] Real-time preview during extraction
- [ ] Support for additional export formats (video, point clouds)
- [ ] Multi-threaded processing for better performance
- [ ] Metadata export (timestamps, camera parameters)
- [ ] Batch scripting support
- [ ] Docker containerization

## 📅 Version History

### v1.0.0 (2025-11-07)
- Initial release
- Basic frame extraction functionality
- Support for left, right, and depth images
- Batch processing capability
- CMake build system
- Comprehensive documentation

---

**Made with ❤️ for the ZED camera community**
