# ZED SVO2 Extraction Tools - Implementation Guide

## Project Overview
This document outlines the implementation of three separate applications for processing ZED 2i SVO2 files.

## Architecture

### Applications
1. **ZED_Video_Extractor** - Extract MP4 videos from SVO2 files
2. **ZED_Frame_Extractor** - Extract frames for YOLO training (1 FPS sampling)
3. **ZED_Depth_Analyzer** - Neural depth analysis with heatmap visualization

### Shared Components (`common/`)
- `metadata.hpp/cpp` - JSON metadata handling
- `gui_base.hpp/cpp` - ImGui setup and utilities
- `svo_handler.hpp/cpp` - Common SVO2 file operations
- `utils.hpp/cpp` - File system utilities

### External Dependencies
- **ZED SDK 4.x** - Camera and SVO2 handling
- **OpenCV 4.x** - Image processing
- **CUDA** - Required by ZED SDK
- **Dear ImGui** - GUI framework
- **GLFW** - Window management for ImGui
- **FFmpeg** - Video encoding (command-line integration)

## Detailed Requirements

### 1. Video Extractor

**GUI Elements:**
- Folder browser button (select flight_YYYYMMDD_HHMMSS folder)
- Display detected SVO2 file
- Display extracted date/time
- Camera selection: Radio buttons (Left, Right, Both)
- When "Both" selected: Checkbox for "Side-by-Side"
- Progress bar with percentage
- Cancel button
- Extract button

**Processing Logic:**
1. Scan selected folder for `.svo` or `.svo2` file
2. Open SVO2 file with ZED SDK
3. Get video properties (resolution, FPS, frame count)
4. Based on selection:
   - **Left/Right**: Extract single camera view, encode to H.264 MP4
   - **Both (Separate)**: Extract and encode both cameras separately
   - **Both (Side-by-Side)**: Combine frames side-by-side, then encode
5. Use FFmpeg for encoding with source FPS and quality settings
6. Generate JSON metadata file

**Output Structure:**
```
E:\Turbulence Solutions\AeroLock\ZED_Recordings_Output\
└── flight_20251105_141806\
    ├── videos\
    │   ├── left.mp4
    │   ├── right.mp4 (if separate)
    │   └── sidebyside.mp4 (if side-by-side)
    └── video_metadata.json
```

### 2. Frame Extractor

**GUI Elements:**
- Folder browser (select flight folder)
- Display SVO2 file info
- Camera selection: Radio buttons (Left, Right)
- Display calculated FPS and frame skip rate
- Display last extracted frame number (if continuing)
- Progress bar
- Cancel button
- Extract button

**Processing Logic:**
1. Check for existing `frame_metadata.json` to get last frame number
2. Calculate frame skip: `skip = round(source_fps / 1.0)` (1 frame per second)
3. Extract frames at calculated rate
4. Save as PNG with naming: `L_frame_000001.png` or `R_frame_000001.png`
5. Continue numbering from last extraction
6. Update JSON metadata

**Output Structure:**
```
E:\Turbulence Solutions\AeroLock\ZED_Recordings_Output\
└── frames\
    ├── L_frame_000001.png
    ├── L_frame_000002.png
    ├── R_frame_000143.png
    └── frame_metadata.json
```

### 3. Depth Analyzer

**GUI Elements:**
- Folder browser
- Display SVO2 file info
- Neural mode dropdown: (NEURAL, NEURAL_PLUS)
- Camera view: Radio buttons (Left, Right)
- Overlay checkbox: "Show camera image"
- Transparency slider: 0-100% (controls heatmap opacity)
- Min object size slider: 50-500 pixels
- Depth range display: 10m (red) to 50m (blue)
- Progress bar
- Cancel button
- Analyze button

**Processing Logic:**
1. Open SVO2 with selected neural depth mode
2. For each frame:
   - Retrieve depth map
   - Filter depth: 10m-50m range (mask out >50m)
   - Find connected components (objects) with minimum pixel count
   - Calculate mean distance for each object
   - Generate heatmap (red=10m, blue=50m gradient)
   - If overlay enabled: blend with camera image at specified transparency
   - Draw bounding boxes around detected objects
   - Display distance in meters (2 decimal places) next to each object
3. Encode frames to MP4 video
4. Calculate statistics (min/max/avg distances, object counts)
5. Generate JSON metadata

**Output Structure:**
```
E:\Turbulence Solutions\AeroLock\ZED_Recordings_Output\
└── flight_20251105_141806\
    └── depth_analysis\
        ├── heatmap_video.mp4
        └── depth_metadata.json
```

## Implementation Phases

### Phase 1: Infrastructure (Current)
- [x] Project structure
- [x] Metadata system
- [ ] GUI base with ImGui
- [ ] SVO handler utilities
- [ ] CMake build system

### Phase 2: Video Extractor
- [ ] GUI implementation
- [ ] SVO2 to frames extraction
- [ ] FFmpeg integration
- [ ] Side-by-side combining
- [ ] Progress tracking

### Phase 3: Frame Extractor
- [ ] GUI implementation
- [ ] FPS calculation and frame skipping
- [ ] Continuous numbering system
- [ ] PNG export

### Phase 4: Depth Analyzer
- [ ] GUI implementation
- [ ] Neural depth extraction
- [ ] Heatmap generation
- [ ] Object detection (connected components)
- [ ] Distance calculation
- [ ] Video encoding with overlays

### Phase 5: Testing & Polish
- [ ] Error handling
- [ ] Performance optimization
- [ ] Documentation
- [ ] User testing

## Technical Notes

### FFmpeg Integration
Use system calls to FFmpeg for video encoding:
```cpp
ffmpeg -framerate {fps} -i frame_%06d.png -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p output.mp4
```

### Heatmap Color Mapping
```cpp
// Map depth (10-50m) to hue (240° blue to 0° red)
float normalizedDepth = (depth - 10.0f) / 40.0f; // 0.0 to 1.0
float hue = (1.0f - normalizedDepth) * 240.0f;   // 240° to 0°
cv::Mat heatmap = applyColorMap(depth_normalized, cv::COLORMAP_JET);
```

### Object Detection
```cpp
// Find connected components in valid depth range
cv::Mat labels, stats, centroids;
int numObjects = cv::connectedComponentsWithStats(validDepthMask, labels, stats, centroids);

// Filter by minimum size
for (int i = 1; i < numObjects; i++) {
    int area = stats.at<int>(i, cv::CC_STAT_AREA);
    if (area >= minObjectPixels) {
        // Calculate mean distance for this object
        // Draw bounding box and distance label
    }
}
```

## Building

See CMakeLists.txt for build instructions. Each application will be built as a separate executable:
- `ZED_Video_Extractor.exe`
- `ZED_Frame_Extractor.exe`
- `ZED_Depth_Analyzer.exe`

All three share the `zed_common` library for metadata and utilities.
