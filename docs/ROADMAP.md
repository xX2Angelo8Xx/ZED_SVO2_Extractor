# ZED SVO2 Extraction Tools - Project Roadmap

**Project Start Date:** November 7, 2025  
**Current Version:** 0.1.0-beta  
**Current Phase:** Phase 2B Complete - GUI Application  
**Project Lead:** Angelo Amon (xX2Angelo8Xx)  
**Repository:** https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor

---

## 🎯 Project Vision

Build professional C++ applications for processing ZED 2i camera SVO2 files:
1. **GUI Extractor** - Modern interface for video and frame extraction ✅
2. **Video Extractor CLI** - Command-line video export ✅
3. **Frame Extractor CLI** - Command-line frame extraction for YOLO training ✅
4. **Depth Analyzer** - Neural depth analysis (planned)

**Key Principles:**
- ✅ Incremental development with stable milestones
- ✅ Comprehensive documentation at every step
- ✅ Version control with clear commit messages
- ✅ Educational approach (learning C++ during development)
- ✅ Professional code quality and comments

---

## 📋 Development Phases

### **Phase 0: Foundation & Setup** ✅ COMPLETE
**Status:** Completed  
**Version:** 0.1.0-alpha  
**Goal:** Establish project structure, documentation, and development workflow

#### Milestones:
- [x] **M0.1:** Initial project structure
- [x] **M0.2:** Metadata system implementation
- [x] **M0.3:** Complete documentation framework
- [x] **M0.4:** Development environment setup guide
- [x] **M0.5:** First successful build

**Completed:** November 7, 2025

---

### **Phase 1: Core Infrastructure** ✅ COMPLETE
**Version:** 0.2.0-alpha  
**Goal:** Build shared components used by all applications

#### Milestones:
- [x] **M1.1:** File system utilities (folder scanning, SVO2 detection, flight parsing)
- [x] **M1.2:** SVO handler (open, read properties, frame retrieval, close)
- [x] **M1.3:** Error handling and logging system
- [x] **M1.4:** Output manager with intelligent path generation
- [x] **M1.5:** Global frame numbering system

#### Key Components:
- `file_utils.cpp/hpp` - File operations and flight folder detection
- `svo_handler.cpp/hpp` - ZED SVO file handling wrapper
- `error_handler.cpp/hpp` - Centralized error codes and logging
- `output_manager.cpp/hpp` - Output path management
- `metadata.cpp/hpp` - Flight metadata and JSON export

**Completed:** November 7, 2025

---

### **Phase 2M1: Metadata System** ✅ COMPLETE
**Version:** 0.2.1-alpha  
**Goal:** Complete metadata extraction and JSON export

#### Milestones:
- [x] **M2M1.1:** Flight folder regex parser (flight_YYYYMMDD_HHMMSS)
- [x] **M2M1.2:** JSON metadata generation
- [x] **M2M1.3:** Camera info extraction from SVO
- [x] **M2M1.4:** Frame counting and duration calculation

**Completed:** November 7, 2025

---

### **Phase 2M2.1: Frame Extractor CLI** ✅ COMPLETE
**Version:** 0.2.2-alpha  
**Goal:** Create command-line frame extractor

#### Milestones:
- [x] **M2.1:** Command-line argument parsing
- [x] **M2.2:** FPS validation and frame interval calculation
- [x] **M2.3:** Frame extraction with global numbering
- [x] **M2.4:** Progress tracking and console output
- [x] **M2.5:** PNG export with YOLO naming (L_frame_XXXXXX.png)
- [x] **M2.6:** Left/Right camera support
- [x] **M2.7:** Flight folder detection and output path management

#### Testing Results:
- ✅ Successfully extracted 716 frames from test SVO
- ✅ Global frame numbering working correctly
- ✅ Flight folder detection validated

**Completed:** November 7, 2025

---

### **Phase 2A: Video Extractor CLI** ✅ COMPLETE
**Version:** 0.2.3-alpha  
**Goal:** Create command-line video extractor

#### Milestones:
- [x] **M2A.1:** Video extraction framework
- [x] **M2A.2:** MJPEG codec integration (AVI container)
- [x] **M2A.3:** Camera mode support (left, right, both_separate, side_by_side)
- [x] **M2A.4:** Quality slider (0-100%)
- [x] **M2A.5:** FPS configuration with validation
- [x] **M2A.6:** Metadata export for video extractions
- [x] **M2A.7:** OpenCV 4.10.0 upgrade (from ZED's bundled 3.1.0)

#### Key Achievements:
- MJPEG codec in AVI container (universally compatible)
- OpenCV 4.10.0 integration with full FFmpeg support
- Automatic FPS validation and fallback to source FPS
- Flight folder detection and extraction numbering

**Completed:** November 7, 2025

---

### **Phase 2B: GUI Application** ✅ COMPLETE
**Version:** 0.1.0-beta  
**Goal:** Modern GUI with real-time progress tracking

#### Milestones:
- [x] **M2B.1:** Dear ImGui + GLFW + OpenGL3 setup
- [x] **M2B.2:** Basic window with file browser (Windows native dialogs)
- [x] **M2B.3:** Frame extraction GUI with settings
- [x] **M2B.4:** Video extraction GUI with camera modes
- [x] **M2B.5:** Real-time progress bars with threading
- [x] **M2B.6:** Extraction engine library with progress callbacks
- [x] **M2B.7:** Cancel button functionality
- [x] **M2B.8:** Result messages and error handling

#### Architecture:
- **Extraction Engine**: Shared library with `std::function` progress callbacks
- **Threading**: `std::thread` for async extraction, atomic flags for cancellation
- **Progress**: Mutex-protected message passing with `std::atomic<float>` progress
- **UI**: Dear ImGui with responsive progress bars and status text

#### Technical Achievements:
- Created `extraction_engine.cpp/hpp` as shared library
- Refactored CLI tools to use extraction engine
- Thread-safe cancellation with `std::atomic<bool>`
- Real-time progress updates without blocking UI
- BGRA to BGR conversion fix for VideoWriter compatibility

**Completed:** November 7, 2025

---

#### Milestones:
- [ ] **M5.1:** GUI layout (reuse Frame Extractor patterns)
- [ ] **M5.2:** Camera mode selection (Left/Right/Both)
- [ ] **M5.3:** Side-by-side checkbox
- [ ] **M5.4:** Video preview (optional)
- [ ] **M5.5:** Progress tracking for video encoding
- [ ] **M5.6:** Output file preview/playback button

#### Learning Focus:
- GUI code reusability
- Complex user input handling
- Process monitoring

**Estimated Duration:** 1 week  
**Target Completion:** Week 10

---

### **Phase 6: Depth Analyzer Foundation (CLI)**
**Version:** 0.7.0-alpha  
**Goal:** Basic depth extraction without object detection

#### Milestones:
- [ ] **M6.1:** Neural depth mode configuration
- [ ] **M6.2:** Depth map extraction
- [ ] **M6.3:** Depth filtering (10-50m range)
- [ ] **M6.4:** Heatmap color mapping (red to blue)
- [ ] **M6.5:** Heatmap image generation
- [ ] **M6.6:** Basic statistics (min/max/avg depth)

#### Learning Focus:
- Computer vision concepts
- Depth sensing principles
- Color space conversions
- Statistical calculations

**Estimated Duration:** 1-2 weeks  
**Target Completion:** Week 12

---

### **Phase 7: Depth Analyzer - Object Detection**
**Completed:** November 7, 2025

---

### **Phase 3: Depth Extraction & Analysis** 🎯 NEXT PHASE
**Version:** 0.2.0-beta  
**Goal:** Add depth map extraction and visualization

#### Planned Milestones:
- [ ] **M3.1:** Depth map extraction (NEURAL mode)
- [ ] **M3.2:** Depth map export (PNG/EXR formats)
- [ ] **M3.3:** Heatmap colorization (10-50m range)
- [ ] **M3.4:** Depth statistics (min/max/mean/median)
- [ ] **M3.5:** GUI integration for depth extraction

#### Future Enhancements:
- Object detection with distance measurement
- Depth video overlay (camera + heatmap blend)
- Connected components analysis for object tracking

**Estimated Duration:** 2-3 weeks  
**Status:** Not started

---

### **Phase 4: Testing & Optimization**
**Version:** 0.3.0-beta  
**Goal:** Comprehensive testing and performance improvements

#### Planned Milestones:
- [ ] **M4.1:** Automated test suite
- [ ] **M4.2:** Performance profiling and optimization
- [ ] **M4.3:** Memory leak detection
- [ ] **M4.4:** Error handling improvements
- [ ] **M4.5:** User acceptance testing

**Estimated Duration:** 1-2 weeks  
**Status:** Not started

---

### **Phase 5: Advanced Features**

- [ ] **M9.2:** Neural mode dropdown
- [ ] **M9.3:** Transparency slider
- [ ] **M9.4:** Min object size slider
- [ ] **M9.5:** Real-time settings preview
- [ ] **M9.6:** Statistics display panel

#### Learning Focus:
- Advanced GUI controls
- Real-time parameter updates
- UI/UX best practices

**Estimated Duration:** 1-2 weeks  
**Target Completion:** Week 19

---

### **Phase 10: Testing & Refinement**
**Version:** 1.0.0-rc1  
**Goal:** Production-ready release candidate

#### Milestones:
- [ ] **M10.1:** Comprehensive testing suite
- [ ] **M10.2:** Error handling improvements
- [ ] **M10.3:** Performance benchmarking
- [ ] **M10.4:** User documentation
- [ ] **M10.5:** Installation package creation
- [ ] **M10.6:** Bug fixes and polish

**Estimated Duration:** 2-3 weeks  
**Target Completion:** Week 22

---

### **Phase 11: Release & Future Enhancements**
**Version:** 1.0.0  
**Goal:** Official release and roadmap for v2.0

#### Milestones:
- [ ] **M11.1:** Official release
- [ ] **M11.2:** User feedback collection
- [ ] **M11.3:** Bug fix releases (1.0.x)
- [ ] **M11.4:** Feature requests prioritization
- [ ] **M11.5:** v2.0 planning

---

## 🏗️ Current Work Breakdown

### **NEXT UP: Phase 0 Completion**

#### Task List:
1. ✅ Create project roadmap (this file)
2. ⏳ Create C++ learning guide
3. ⏳ Write dependency setup guide
4. ⏳ Create code style guide
5. ⏳ Write git workflow documentation
6. ⏳ Setup CMakeLists.txt structure
7. ⏳ Create first buildable (empty) applications

---

## 📊 Version Numbering

**Format:** MAJOR.MINOR.PATCH-stage

- **MAJOR:** Incompatible changes (1.x.x → 2.x.x)
- **MINOR:** New features, backward compatible (0.2.x → 0.3.x)
- **PATCH:** Bug fixes (0.1.0 → 0.1.1)
- **Stage:** alpha → beta → rc (release candidate) → (none for stable)

### Version History:
- **0.1.0-alpha** (Current) - Project structure and metadata system

---

## 🎓 Learning Path

Each phase introduces new C++ concepts:

| Phase | C++ Topics |
|-------|------------|
| 0-1 | Project structure, includes, namespaces, basic syntax |
| 2 | File I/O, strings, vectors, loops |
| 3 | Pointers, references, classes, objects |
| 4 | Memory management, RAII, smart pointers |
| 5 | Templates, STL containers, algorithms |
| 6-9 | Advanced OOP, design patterns, optimization |

---

## 📝 Documentation Structure

```
docs/
├── ROADMAP.md (this file)
├── CPP_LEARNING_GUIDE.md
├── DEPENDENCY_SETUP.md
├── CODE_STYLE_GUIDE.md
├── GIT_WORKFLOW.md
├── IMPLEMENTATION.md
├── API_REFERENCE.md (future)
└── USER_MANUAL.md (future)
```

---

## 🔄 Development Workflow

1. **Start of Session:**
   - Review current phase and milestones
   - Check out working branch (if needed)
   - Review last commit

2. **During Development:**
   - Work on ONE milestone at a time
   - Commit frequently with clear messages
   - Update documentation as you code
   - Test before committing

3. **End of Session:**
   - Commit all changes
   - Update roadmap with progress
   - Push to GitHub
   - Document learnings/challenges

---

## 🚀 Quick Start (Once Setup Complete)

### Build All Applications:
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Run Applications:
```bash
./ZED_Frame_Extractor --help
./ZED_Video_Extractor --help
./ZED_Depth_Analyzer --help
```

---

## 📞 Support & Resources

- **ZED SDK Docs:** https://www.stereolabs.com/docs/
- **OpenCV Docs:** https://docs.opencv.org/
- **ImGui Docs:** https://github.com/ocornut/imgui
- **C++ Reference:** https://en.cppreference.com/

---

## 📅 Progress Tracking

**Last Updated:** November 7, 2025  
**Current Phase:** Phase 0 (Foundation)  
**Completion:** 40%  
**Next Milestone:** M0.3 - Complete documentation framework

---

*This roadmap is a living document and will be updated as the project evolves.*
