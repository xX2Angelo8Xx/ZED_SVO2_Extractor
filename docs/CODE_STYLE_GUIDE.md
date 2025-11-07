# Code Style Guide

**Purpose:** Maintain consistent, readable code throughout the project  
**Standard:** Based on Google C++ Style Guide with project-specific adaptations

---

## 📋 General Principles

1. **Clarity over cleverness** - Code should be easy to understand
2. **Consistency** - Follow existing patterns in the codebase
3. **Comments** - Explain WHY, not WHAT (code shows what)
4. **Documentation** - Every public function needs documentation
5. **Error handling** - Always check for errors and handle gracefully

---

## 📁 File Organization

### Header Files (.hpp)

```cpp
/**
 * @file frame_extractor.hpp
 * @brief Frame extraction utilities for ZED SVO2 files
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#pragma once

// System includes (alphabetically)
#include <memory>
#include <string>
#include <vector>

// Third-party includes (alphabetically)
#include <opencv2/opencv.hpp>
#include <sl/Camera.hpp>

// Project includes (alphabetically)
#include "metadata.hpp"
#include "utils.hpp"

// Namespace
namespace zed_tools {

// Class declarations, function prototypes, etc.

} // namespace zed_tools
```

### Source Files (.cpp)

```cpp
/**
 * @file frame_extractor.cpp
 * @brief Implementation of frame extraction utilities
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "frame_extractor.hpp"

// Additional includes needed for implementation
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace zed_tools {

// Implementation

} // namespace zed_tools
```

---

## 🏷️ Naming Conventions

### Files
- **Lower case with underscores:** `frame_extractor.cpp`, `metadata.hpp`
- **Match class name:** `FlightInfo` class → `flight_info.hpp`

### Classes & Structs
- **PascalCase:** `VideoMetadata`, `FlightInfo`, `FrameExtractor`

```cpp
class VideoMetadata {
    // ...
};

struct FlightInfo {
    // ...
};
```

### Functions & Methods
- **camelCase:** `saveToJSON()`, `calculateFrameSkip()`, `parseFromFolder()`

```cpp
bool saveToJSON(const std::string& path) const;
int calculateFrameSkip(double fps, int targetFps);
```

### Variables
- **camelCase:** `frameCount`, `outputPath`, `isValid`

```cpp
int frameCount = 0;
std::string outputPath = "output/";
bool isValid = true;
```

### Constants
- **ALL_CAPS with underscores:** `MAX_FRAMES`, `DEFAULT_FPS`

```cpp
const int MAX_FRAMES = 10000;
const double DEFAULT_FPS = 30.0;
```

### Member Variables
- **camelCase with no prefix:**

```cpp
class VideoExtractor {
private:
    int frameCount;      // Good
    std::string path;    // Good
    bool isRunning;      // Good
    
    // Not: m_frameCount, _frameCount, frameCount_
};
```

### Namespaces
- **Lower case with underscores:** `zed_tools`, `depth_analysis`

```cpp
namespace zed_tools {
    // ...
}
```

---

## 📝 Comments & Documentation

### File Headers
Every file starts with:

```cpp
/**
 * @file filename.cpp
 * @brief Brief description of file purpose
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */
```

### Class Documentation

```cpp
/**
 * @brief Handles extraction of frames from SVO2 files
 * 
 * This class provides methods to open SVO2 files, extract individual frames,
 * and save them as PNG images with proper naming conventions.
 * 
 * Example usage:
 * @code
 * FrameExtractor extractor;
 * extractor.openSVO("path/to/file.svo2");
 * extractor.extractFrame(0, "output/frame_000001.png");
 * @endcode
 */
class FrameExtractor {
    // ...
};
```

### Function Documentation

```cpp
/**
 * @brief Calculate frame skip rate for target FPS
 * 
 * Determines how many frames to skip to achieve the desired extraction
 * rate. For example, if source is 30fps and target is 1fps, returns 30.
 * 
 * @param sourceFps Original video frame rate
 * @param targetFps Desired extraction frame rate
 * @return Number of frames to skip between extractions
 * 
 * @note Returns 1 if calculation results in 0 or negative value
 * @warning targetFps must be positive and less than sourceFps
 */
int calculateFrameSkip(double sourceFps, double targetFps);
```

### Inline Comments

```cpp
// Good: Explains WHY
// Skip corrupted frames at the end of recording
if (frameIndex > validFrameCount) {
    continue;
}

// Bad: States the obvious
// Increment i
i++;

// Good: Complex logic explanation
// Use inverse mapping for heatmap: close objects (10m) = red (0°),
// far objects (50m) = blue (240°) in HSV color space
float hue = (1.0f - normalizedDepth) * 240.0f;
```

### TODO Comments

```cpp
// TODO(angelo): Add support for H.265 encoding
// TODO: Implement frame caching for better performance
// FIXME: Memory leak in depth map processing
// NOTE: This assumes left-handed coordinate system
```

---

## 🔤 Formatting

### Indentation
- **4 spaces** (no tabs)
- Configure your editor:

**VS Code (`.vscode/settings.json`):**
```json
{
    "editor.tabSize": 4,
    "editor.insertSpaces": true
}
```

### Braces
- **Opening brace on same line** (K&R style)

```cpp
// Good
if (condition) {
    doSomething();
}

void function() {
    // code
}

// Not recommended
if (condition)
{
    doSomething();
}
```

### Line Length
- **Maximum 100 characters**
- Break long lines logically:

```cpp
// Good
bool result = extractFrame(
    inputPath,
    outputPath,
    frameNumber,
    cameraMode
);

// Good
std::cout << "Processing frame " << frameNumber 
          << " of " << totalFrames << std::endl;
```

### Spacing

```cpp
// Around operators
int x = 5 + 3;
bool result = (a == b) && (c != d);

// After commas
function(param1, param2, param3);

// Not inside parentheses
if (condition) {        // Good
if ( condition ) {      // Bad

// Function calls
doSomething(x, y);      // Good
doSomething (x, y);     // Bad

// Template brackets
std::vector<int> vec;   // Good
std::vector <int> vec;  // Bad
```

### Empty Lines

```cpp
class Example {
public:
    // Constructor
    Example();
    
    // Main methods
    void initialize();
    void process();
    
    // Helper methods
    void cleanup();
    
private:
    int data;
};

// Function implementation
void Example::process() {
    // Logical block 1
    setupData();
    validateInput();
    
    // Logical block 2
    performCalculation();
    saveResults();
    
    // Logical block 3
    cleanup();
}
```

---

## 🎯 Code Patterns

### Error Handling

```cpp
// Pattern 1: Return bool for success/failure
bool processFile(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "ERROR: File not found: " << path << std::endl;
        return false;
    }
    
    // Process...
    
    return true;
}

// Pattern 2: Throw exceptions for exceptional cases
void openSVO(const std::string& path) {
    if (!fs::exists(path)) {
        throw std::runtime_error("SVO file not found: " + path);
    }
    
    // Open file...
}

// Usage
try {
    openSVO(path);
} catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
}
```

### Resource Management (RAII)

```cpp
// Good: RAII with smart pointers
std::unique_ptr<FrameExtractor> extractor = 
    std::make_unique<FrameExtractor>();
// Automatically cleaned up

// Good: RAII with standard library
{
    std::ofstream file("output.txt");
    file << "data";
    // Automatically closed at end of scope
}

// Bad: Manual management
FrameExtractor* extractor = new FrameExtractor();
// ... easy to forget:
delete extractor;
```

### Const Correctness

```cpp
class VideoMetadata {
public:
    // Const member function (doesn't modify object)
    bool saveToJSON(const std::string& path) const;
    
    // Const reference parameter (efficient, read-only)
    void setTitle(const std::string& title);
    
    // Const pointer (pointer itself is const)
    void process(const int* data);
    
private:
    // Const member (never changes)
    const int MAX_SIZE = 1000;
};
```

### Namespace Usage

```cpp
// In header:
namespace zed_tools {
    class VideoExtractor { /*...*/ };
}

// In implementation:
namespace zed_tools {
    void VideoExtractor::process() {
        // Implementation
    }
}

// Usage:
zed_tools::VideoExtractor extractor;

// Or with using:
using zed_tools::VideoExtractor;
VideoExtractor extractor;

// Or entire namespace (only in .cpp, never in .hpp):
using namespace zed_tools;
VideoExtractor extractor;
```

---

## ✅ Modern C++ Practices

### Use C++17 Features

```cpp
// Filesystem library
#include <filesystem>
namespace fs = std::filesystem;

fs::path outputPath = "output/frames";
if (fs::exists(outputPath)) {
    fs::create_directories(outputPath);
}

// Structured bindings
std::map<std::string, int> data;
for (const auto& [key, value] : data) {
    std::cout << key << ": " << value << std::endl;
}

// std::optional for optional values
std::optional<int> findFrame(int id) {
    if (exists(id)) {
        return id;
    }
    return std::nullopt;
}
```

### Prefer Auto When Appropriate

```cpp
// Good: Type is obvious or complex
auto extractor = std::make_unique<FrameExtractor>();
auto it = vectorData.begin();
auto [min, max] = std::minmax({1, 5, 3, 9, 2});

// Bad: Type is not obvious
auto x = getData();  // What type is x?

// Better:
int x = getData();  // Clear!
```

### Range-Based For Loops

```cpp
std::vector<std::string> files = {"a.png", "b.png", "c.png"};

// Good: Range-based
for (const auto& file : files) {
    process(file);
}

// Old style (only when you need index):
for (size_t i = 0; i < files.size(); ++i) {
    std::cout << i << ": " << files[i] << std::endl;
}
```

---

## 🚫 Common Mistakes to Avoid

### Memory Leaks

```cpp
// Bad
int* data = new int[1000];
// ... forgot to delete

// Good
std::vector<int> data(1000);
// Automatically managed
```

### Raw Pointers

```cpp
// Bad: Raw owning pointer
Widget* widget = new Widget();
delete widget;  // Easy to forget!

// Good: Smart pointer
auto widget = std::make_unique<Widget>();
// Automatic cleanup
```

### Magic Numbers

```cpp
// Bad
if (distance > 50) {
    // What is 50?
}

// Good
const float MAX_DETECTION_RANGE_METERS = 50.0f;
if (distance > MAX_DETECTION_RANGE_METERS) {
    // Clear meaning
}
```

### String Concatenation in Loops

```cpp
// Bad: Inefficient
std::string result;
for (int i = 0; i < 1000; ++i) {
    result += std::to_string(i);  // Creates temporary each time
}

// Good: Use stringstream
std::stringstream ss;
for (int i = 0; i < 1000; ++i) {
    ss << i;
}
std::string result = ss.str();
```

---

## 📊 Example: Well-Formatted Class

```cpp
/**
 * @file frame_extractor.hpp
 * @brief Frame extraction from ZED SVO2 files
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <sl/Camera.hpp>

#include "metadata.hpp"

namespace zed_tools {

/**
 * @brief Extracts frames from SVO2 files for YOLO training
 * 
 * Handles opening SVO2 files, calculating optimal frame skip rates,
 * and exporting frames as PNG images with continuous numbering.
 */
class FrameExtractor {
public:
    /**
     * @brief Construct a new Frame Extractor
     */
    FrameExtractor();
    
    /**
     * @brief Destroy the Frame Extractor
     */
    ~FrameExtractor();
    
    /**
     * @brief Open an SVO2 file for processing
     * 
     * @param svoPath Path to the SVO2 file
     * @return true if file opened successfully
     * @return false if file not found or invalid
     */
    bool openSVO(const std::string& svoPath);
    
    /**
     * @brief Extract a single frame and save as PNG
     * 
     * @param frameIndex Zero-based frame index
     * @param outputPath Where to save the PNG file
     * @param camera Which camera view (LEFT or RIGHT)
     * @return true if extraction successful
     */
    bool extractFrame(
        int frameIndex,
        const std::string& outputPath,
        sl::VIEW camera = sl::VIEW::LEFT
    );
    
    /**
     * @brief Get the frames per second of the opened SVO
     * 
     * @return double FPS value, or 0.0 if no file is open
     */
    double getFPS() const;
    
private:
    std::unique_ptr<sl::Camera> zedCamera;
    FrameMetadata metadata;
    bool isOpen;
    
    /**
     * @brief Calculate next frame number for continuous numbering
     * 
     * Checks existing frame metadata to determine where to continue
     * numbering from previous extraction sessions.
     * 
     * @return int Next frame number to use
     */
    int calculateNextFrameNumber();
};

} // namespace zed_tools
```

---

## ✅ Checklist Before Committing

- [ ] Code compiles without warnings
- [ ] All functions have documentation comments
- [ ] Variable names are descriptive
- [ ] No magic numbers (use constants)
- [ ] Error cases are handled
- [ ] Resources are properly managed (RAII)
- [ ] Code follows formatting guidelines
- [ ] No commented-out code (use git history instead)
- [ ] TODO comments have author name
- [ ] File has proper header comment

---

## 🔧 Editor Configuration

### VS Code `.vscode/settings.json`

```json
{
    "editor.tabSize": 4,
    "editor.insertSpaces": true,
    "editor.formatOnSave": true,
    "editor.rulers": [100],
    "files.trimTrailingWhitespace": true,
    "files.insertFinalNewline": true,
    "C_Cpp.clang_format_style": "{ BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 100 }"
}
```

### Visual Studio

- Tools → Options → Text Editor → C/C++ → Tabs
  - Tab size: 4
  - Indent size: 4
  - Insert spaces: ✓

---

*Consistency is key! Follow these guidelines for clean, maintainable code.*
