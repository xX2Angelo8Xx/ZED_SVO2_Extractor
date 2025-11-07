# ZED SVO2 Extraction Tools - Project Roadmap

**Project Start Date:** November 7, 2025  
**Current Version:** 0.1.0-alpha  
**Project Lead:** Angelo Amon (xX2Angelo8Xx)  
**Repository:** https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor

---

## 🎯 Project Vision

Build three professional C++ applications for processing ZED 2i camera SVO2 files:
1. **Video Extractor** - Export MP4 videos
2. **Frame Extractor** - Extract frames for YOLO model training
3. **Depth Analyzer** - Neural depth analysis with heatmap visualization

**Key Principles:**
- ✅ Incremental development with stable milestones
- ✅ Comprehensive documentation at every step
- ✅ Version control with clear commit messages
- ✅ Educational approach (learning C++ during development)
- ✅ Professional code quality and comments

---

## 📋 Development Phases

### **Phase 0: Foundation & Setup** ✅ CURRENT PHASE
**Status:** In Progress  
**Version:** 0.1.0-alpha  
**Goal:** Establish project structure, documentation, and development workflow

#### Milestones:
- [x] **M0.1:** Initial project structure
- [x] **M0.2:** Metadata system implementation
- [ ] **M0.3:** Complete documentation framework
- [ ] **M0.4:** Development environment setup guide
- [ ] **M0.5:** First successful build of empty applications

#### Deliverables:
- Project roadmap (this document)
- C++ learning guide
- Dependency installation guide
- Code style guidelines
- Git workflow documentation
- CMakeLists.txt with build structure

**Target Completion:** Week 1

---

### **Phase 1: Core Infrastructure**
**Version:** 0.2.0-alpha  
**Goal:** Build shared components used by all three applications

#### Milestones:
- [ ] **M1.1:** File system utilities (folder scanning, SVO2 detection)
- [ ] **M1.2:** Basic SVO2 handler (open, read properties, close)
- [ ] **M1.3:** Error handling and logging system
- [ ] **M1.4:** Progress tracking system
- [ ] **M1.5:** Unit tests for core utilities

#### Learning Focus:
- C++ file I/O and filesystem library
- ZED SDK basics
- Smart pointers and RAII
- Error handling patterns

**Estimated Duration:** 1-2 weeks  
**Target Completion:** Week 3

---

### **Phase 2: Frame Extractor (CLI)** 🎯 FIRST APPLICATION
**Version:** 0.3.0-alpha  
**Goal:** Create command-line frame extractor (simplest application)

#### Milestones:
- [ ] **M2.1:** Command-line argument parsing
- [ ] **M2.2:** FPS detection and frame skip calculation
- [ ] **M2.3:** Single frame extraction (left camera)
- [ ] **M2.4:** Batch frame extraction with progress display
- [ ] **M2.5:** PNG export with proper naming (L_frame_XXXXXX.png)
- [ ] **M2.6:** Continuous numbering system
- [ ] **M2.7:** JSON metadata generation
- [ ] **M2.8:** Right camera support

#### Learning Focus:
- Image processing with OpenCV
- File naming and numbering logic
- Console output formatting
- JSON file writing

**Estimated Duration:** 1-2 weeks  
**Target Completion:** Week 5

---

### **Phase 3: Frame Extractor GUI**
**Version:** 0.4.0-beta  
**Goal:** Add GUI to Frame Extractor

#### Milestones:
- [ ] **M3.1:** ImGui + GLFW setup
- [ ] **M3.2:** Basic window with file browser
- [ ] **M3.3:** Camera selection radio buttons
- [ ] **M3.4:** Progress bar integration
- [ ] **M3.5:** Cancel button functionality
- [ ] **M3.6:** Settings display (FPS, frame skip, etc.)
- [ ] **M3.7:** Error dialogs

#### Learning Focus:
- GUI programming concepts
- Event-driven programming
- Threading (UI vs processing thread)
- ImGui framework

**Estimated Duration:** 1-2 weeks  
**Target Completion:** Week 7

---

### **Phase 4: Video Extractor (CLI)**
**Version:** 0.5.0-alpha  
**Goal:** Create command-line video extractor

#### Milestones:
- [ ] **M4.1:** Frame extraction for video (single camera)
- [ ] **M4.2:** FFmpeg integration (system call)
- [ ] **M4.3:** H.264 encoding with quality settings
- [ ] **M4.4:** Side-by-side frame combining
- [ ] **M4.5:** Dual camera extraction (separate files)
- [ ] **M4.6:** Video metadata generation
- [ ] **M4.7:** Cleanup of temporary frame files

#### Learning Focus:
- Video encoding concepts
- FFmpeg command-line usage
- Image concatenation
- Temporary file management

**Estimated Duration:** 1-2 weeks  
**Target Completion:** Week 9

---

### **Phase 5: Video Extractor GUI**
**Version:** 0.6.0-beta  
**Goal:** Add GUI to Video Extractor

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
**Version:** 0.8.0-alpha  
**Goal:** Add object detection and distance measurement

#### Milestones:
- [ ] **M7.1:** Connected components algorithm
- [ ] **M7.2:** Object filtering by size
- [ ] **M7.3:** Mean distance calculation per object
- [ ] **M7.4:** Bounding box drawing
- [ ] **M7.5:** Distance label rendering
- [ ] **M7.6:** Object tracking across frames (optional)

#### Learning Focus:
- Image segmentation
- Connected components analysis
- Text rendering in OpenCV
- Object tracking algorithms

**Estimated Duration:** 2-3 weeks  
**Target Completion:** Week 15

---

### **Phase 8: Depth Analyzer - Video & Overlay**
**Version:** 0.9.0-alpha  
**Goal:** Video output with camera overlay

#### Milestones:
- [ ] **M8.1:** Camera image overlay
- [ ] **M8.2:** Transparency blending
- [ ] **M8.3:** Video encoding with overlays
- [ ] **M8.4:** Frame-by-frame processing
- [ ] **M8.5:** Performance optimization
- [ ] **M8.6:** Complete statistics generation

#### Learning Focus:
- Image blending algorithms
- Alpha compositing
- Video processing optimization
- Memory management

**Estimated Duration:** 1-2 weeks  
**Target Completion:** Week 17

---

### **Phase 9: Depth Analyzer GUI**
**Version:** 0.10.0-beta  
**Goal:** Complete GUI for Depth Analyzer

#### Milestones:
- [ ] **M9.1:** Complex GUI layout
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
