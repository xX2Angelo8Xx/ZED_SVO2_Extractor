# C++ Learning Guide for ZED SVO2 Project

**Purpose:** Learn C++ through building real applications  
**Approach:** Practical examples from our actual codebase  
**Level:** Beginner to Intermediate

---

## 📚 Table of Contents

1. [C++ Basics](#cpp-basics)
2. [Project Structure](#project-structure)
3. [Key Concepts by Phase](#key-concepts-by-phase)
4. [Code Examples from Our Project](#code-examples)
5. [Common Patterns](#common-patterns)
6. [Debugging Tips](#debugging-tips)
7. [Resources](#resources)

---

## 🎯 C++ Basics

### What is C++?

C++ is a compiled, statically-typed language that gives you:
- **Performance:** Direct hardware control
- **Memory Management:** Control over how data is stored
- **Object-Oriented:** Classes and objects for organization
- **Standard Library:** Powerful built-in tools

### Compilation Process

```
Source Code (.cpp, .hpp)
    ↓ [Compiler]
Object Files (.o, .obj)
    ↓ [Linker]
Executable (.exe)
```

---

## 📁 Project Structure

### Header Files (.hpp, .h)

**Purpose:** Declare what exists (function signatures, class definitions)

```cpp
// metadata.hpp
#pragma once  // Prevent multiple includes

namespace zed_tools {
    // Declaration: "This class exists"
    class VideoMetadata {
    public:
        std::string extractionDateTime;
        int width;
        int height;
        
        bool saveToJSON(const std::string& path) const;
    };
}
```

### Source Files (.cpp)

**Purpose:** Define how things work (implementation)

```cpp
// metadata.cpp
#include "metadata.hpp"

namespace zed_tools {
    // Implementation: "Here's how saveToJSON works"
    bool VideoMetadata::saveToJSON(const std::string& path) const {
        // Actual code here
        return true;
    }
}
```

### Why Separate?

- **Headers:** Quick reference, shared between files
- **Source:** Detailed implementation, compiled once
- **Benefit:** Faster compilation, cleaner code organization

---

## 🔑 Key Concepts by Phase

### Phase 0-1: Foundation

#### 1. **Namespaces**

Prevent name collisions:

```cpp
namespace zed_tools {
    class VideoMetadata { /* ... */ };
}

namespace other_library {
    class VideoMetadata { /* ... */ };  // No conflict!
}

// Usage:
zed_tools::VideoMetadata myData;
```

#### 2. **Include Guards**

Prevent duplicate definitions:

```cpp
// Old style:
#ifndef METADATA_HPP
#define METADATA_HPP
// ... code ...
#endif

// Modern style (preferred):
#pragma once
// ... code ...
```

#### 3. **Basic Data Types**

```cpp
int frameCount = 0;           // Integer
float distance = 15.5f;       // Floating point (4 bytes)
double preciseValue = 15.5;   // Double precision (8 bytes)
bool isValid = true;          // Boolean
char letter = 'A';            // Single character
std::string name = "Angelo";  // String (needs #include <string>)
```

#### 4. **Variables & Constants**

```cpp
// Variable (can change)
int counter = 0;
counter = 5;  // OK

// Constant (cannot change)
const int MAX_FRAMES = 1000;
// MAX_FRAMES = 2000;  // ERROR!

// Constant reference (common in function parameters)
void processData(const std::string& filename) {
    // filename cannot be modified here
}
```

---

### Phase 2: Frame Extractor Basics

#### 5. **Functions**

```cpp
// Declaration in .hpp
int calculateFrameSkip(double fps, int targetFps);

// Implementation in .cpp
int calculateFrameSkip(double fps, int targetFps) {
    return static_cast<int>(std::round(fps / targetFps));
}

// Usage:
int skip = calculateFrameSkip(30.0, 1);  // Returns 30
```

#### 6. **Strings**

```cpp
#include <string>

std::string filename = "frame_";
filename += "00001";           // Concatenation
filename = filename + ".png";  // Result: "frame_00001.png"

// String length
int len = filename.length();

// Substring
std::string extension = filename.substr(filename.length() - 3);  // "png"

// Find
size_t pos = filename.find(".");  // Position of first "."

// Comparison
if (filename == "frame_00001.png") {
    // ...
}
```

#### 7. **Vectors (Dynamic Arrays)**

```cpp
#include <vector>

// Create empty vector
std::vector<std::string> filenames;

// Add elements
filenames.push_back("file1.png");
filenames.push_back("file2.png");

// Access elements
std::string first = filenames[0];  // "file1.png"

// Size
int count = filenames.size();  // 2

// Iterate
for (const auto& filename : filenames) {
    std::cout << filename << std::endl;
}

// Iterate with index
for (size_t i = 0; i < filenames.size(); ++i) {
    std::cout << i << ": " << filenames[i] << std::endl;
}
```

#### 8. **File I/O**

```cpp
#include <fstream>
#include <iostream>

// Writing to file
std::ofstream outFile("output.txt");
if (outFile.is_open()) {
    outFile << "Hello, World!" << std::endl;
    outFile << "Frame count: " << 100 << std::endl;
    outFile.close();
} else {
    std::cerr << "Failed to open file" << std::endl;
}

// Reading from file
std::ifstream inFile("input.txt");
std::string line;
if (inFile.is_open()) {
    while (std::getline(inFile, line)) {
        std::cout << line << std::endl;
    }
    inFile.close();
}
```

---

### Phase 3: GUI & Classes

#### 9. **Classes**

```cpp
// metadata.hpp
class FlightInfo {
public:
    // Public: accessible from outside
    std::string folderName;
    std::string date;
    
    // Constructor (called when object is created)
    FlightInfo() : folderName(""), date("") {}
    
    // Member function
    bool parseFromFolder(const std::string& path);
    
private:
    // Private: only accessible within class
    std::string internalCache;
};

// Usage:
FlightInfo info;
info.folderName = "flight_20251105_141806";
bool success = info.parseFromFolder("/path/to/folder");
```

#### 10. **Pointers & References**

```cpp
// Value (copy)
int x = 5;
int y = x;  // y is a COPY of x
y = 10;     // x is still 5

// Reference (alias)
int x = 5;
int& y = x;  // y is an ALIAS for x
y = 10;      // x is now also 10!

// Pointer (memory address)
int x = 5;
int* ptr = &x;  // ptr holds the ADDRESS of x
*ptr = 10;      // Changes x to 10 (dereference)

// Common usage in functions
void modifyValue(int value) {
    value = 10;  // Doesn't affect original
}

void modifyReference(int& value) {
    value = 10;  // DOES affect original
}

void modifyPointer(int* value) {
    *value = 10;  // DOES affect original
}

int x = 5;
modifyValue(x);      // x is still 5
modifyReference(x);  // x is now 10
modifyPointer(&x);   // x is now 10
```

#### 11. **Smart Pointers (C++11+)**

Automatic memory management:

```cpp
#include <memory>

// unique_ptr: single owner
std::unique_ptr<FlightInfo> info = std::make_unique<FlightInfo>();
info->folderName = "flight_20251105_141806";
// Automatically deleted when out of scope

// shared_ptr: multiple owners
std::shared_ptr<VideoMetadata> metadata = std::make_shared<VideoMetadata>();
std::shared_ptr<VideoMetadata> copy = metadata;  // Both point to same object
// Deleted when last shared_ptr goes out of scope
```

---

### Phase 4-5: Advanced Concepts

#### 12. **Templates**

Generic programming:

```cpp
// Function template
template<typename T>
T maximum(T a, T b) {
    return (a > b) ? a : b;
}

// Usage:
int maxInt = maximum(5, 10);           // Returns 10
double maxDouble = maximum(3.5, 2.1);  // Returns 3.5
```

#### 13. **STL Algorithms**

```cpp
#include <algorithm>
#include <vector>

std::vector<int> numbers = {5, 2, 8, 1, 9};

// Sort
std::sort(numbers.begin(), numbers.end());
// Result: {1, 2, 5, 8, 9}

// Find
auto it = std::find(numbers.begin(), numbers.end(), 5);
if (it != numbers.end()) {
    std::cout << "Found at position: " 
              << std::distance(numbers.begin(), it) << std::endl;
}

// Count
int count = std::count(numbers.begin(), numbers.end(), 5);

// Transform
std::vector<int> doubled(numbers.size());
std::transform(numbers.begin(), numbers.end(), doubled.begin(),
               [](int n) { return n * 2; });
// Result: {2, 4, 10, 16, 18}
```

---

## 💡 Code Examples from Our Project

### Example 1: FlightInfo Parser

```cpp
bool FlightInfo::parseFromFolder(const std::string& folderPath) {
    // Extract folder name from full path
    fs::path path(folderPath);
    folderName = path.filename().string();
    
    // Regular expression to match: flight_YYYYMMDD_HHMMSS
    std::regex pattern(R"(flight_(\d{8})_(\d{6}))");
    std::smatch matches;
    
    if (std::regex_search(folderName, matches, pattern)) {
        if (matches.size() == 3) {
            std::string dateStr = matches[1].str();  // YYYYMMDD
            std::string timeStr = matches[2].str();  // HHMMSS
            
            // Format date as YYYY-MM-DD
            date = dateStr.substr(0, 4) + "-" + 
                   dateStr.substr(4, 2) + "-" + 
                   dateStr.substr(6, 2);
            
            // Format time as HH:MM:SS
            time = timeStr.substr(0, 2) + ":" + 
                   timeStr.substr(2, 2) + ":" + 
                   timeStr.substr(4, 2);
            
            return true;
        }
    }
    
    return false;
}
```

**Key Concepts:**
- Filesystem library (`fs::path`)
- Regular expressions (`std::regex`)
- String manipulation (`substr`)
- Conditional logic

### Example 2: JSON Builder

```cpp
void JSONBuilder::addString(const std::string& key, const std::string& value) {
    addCommaIfNeeded();    // Add comma if not first element
    addIndent();           // Add proper indentation
    ss << "\"" << key << "\": \"" << value << "\"";
}

// Usage:
JSONBuilder json;
json.beginObject();
json.addString("name", "Angelo");
json.addNumber("age", 25);
json.endObject();

std::string result = json.toString();
// Result: { "name": "Angelo", "age": 25 }
```

**Key Concepts:**
- Classes and member functions
- String streams (`std::stringstream`)
- Method chaining
- State management

---

## 🎨 Common Patterns

### Pattern 1: RAII (Resource Acquisition Is Initialization)

```cpp
// BAD: Manual resource management
void processFile() {
    FILE* file = fopen("data.txt", "r");
    // ... do work ...
    fclose(file);  // Easy to forget!
}

// GOOD: RAII with std::ifstream
void processFile() {
    std::ifstream file("data.txt");
    // ... do work ...
    // File automatically closed when out of scope
}
```

### Pattern 2: Error Handling

```cpp
// Return bool for success/failure
bool saveToFile(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open file: " << path << std::endl;
        return false;
    }
    
    file << "data";
    return true;
}

// Usage:
if (!saveToFile("output.txt")) {
    // Handle error
}
```

### Pattern 3: Progress Tracking

```cpp
void processFrames(int totalFrames) {
    for (int i = 0; i < totalFrames; ++i) {
        // Process frame
        
        // Report progress every 10 frames
        if (i % 10 == 0) {
            float progress = (float)i / (float)totalFrames * 100.0f;
            std::cout << "Progress: " << progress << "%" << std::endl;
        }
    }
}
```

---

## 🐛 Debugging Tips

### 1. Compilation Errors

```cpp
// ERROR: undefined reference to `FlightInfo::parseFromFolder`
// CAUSE: Declared in .hpp but not implemented in .cpp
// FIX: Implement the function in metadata.cpp
```

### 2. Segmentation Fault

```cpp
// Common causes:
int* ptr = nullptr;
*ptr = 5;  // CRASH! Dereferencing null pointer

std::vector<int> vec;
int x = vec[0];  // CRASH! Accessing empty vector

// Prevention:
if (ptr != nullptr) {
    *ptr = 5;
}

if (!vec.empty()) {
    int x = vec[0];
}
```

### 3. Debugging Output

```cpp
#include <iostream>

std::cout << "DEBUG: frameCount = " << frameCount << std::endl;
std::cout << "DEBUG: Entering function processFrame" << std::endl;

// Or use std::cerr for errors
std::cerr << "ERROR: Invalid frame number: " << frameNum << std::endl;
```

---

## 📚 Resources

### Official Documentation
- **C++ Reference:** https://en.cppreference.com/
- **LearnCpp.com:** https://www.learncpp.com/
- **CPlusPlus.com:** https://cplusplus.com/

### Video Tutorials
- **The Cherno C++:** https://youtube.com/c/TheChernoProject
- **CppCon Talks:** https://youtube.com/user/CppCon

### Books (Recommended)
- "C++ Primer" by Stanley Lippman
- "Effective Modern C++" by Scott Meyers

### Project-Specific
- **ZED SDK:** https://www.stereolabs.com/docs/
- **OpenCV:** https://docs.opencv.org/
- **CMake:** https://cmake.org/documentation/

---

## 🎓 Learning Strategy for This Project

1. **Start Small:** Read existing code before writing new code
2. **Experiment:** Modify examples and see what happens
3. **Ask Questions:** Use comments to mark confusing parts
4. **Debug Often:** Add print statements to understand flow
5. **Reference Docs:** Look up unfamiliar functions
6. **Commit Frequently:** Version control is your safety net

---

## 📝 Next Steps

1. Read through this guide
2. Review `common/metadata.cpp` to see real examples
3. Try modifying small parts of the code
4. Ask questions about anything unclear
5. Ready to start Phase 1!

---

*This guide will be updated as we progress through the project.*
