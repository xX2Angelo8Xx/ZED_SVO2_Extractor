# Depth Visualization Pipeline

This document captures the current implementation and planned enhancements for depth map visualization and heatmap generation. It complements the extraction engine documentation.

## Goals
- Preserve small, thin distant objects (e.g., model airplane)
- Suppress noisy / low-confidence background
- Provide adjustable contrast without destroying metric meaning
- Support overlay on RGB for contextual recognition

## Current Implemented Pipeline (2025-11-07)
1. Configuration (`DepthExtractionConfig`):
   - `minDepth`, `maxDepth` (default 10–40 m)
   - `overlayOnRgb`, `overlayStrength` (0=RGB only, 100=heatmap only)
   - `autoContrast` (percentile scaling on per-frame valid pixels)
   - `confidenceThreshold` (mask out low-confidence pixels; also applied to runtime parameters)
2. Frame Retrieval:
   - Depth: `sl::MEASURE::DEPTH` (32F meters)
   - Confidence: `sl::MEASURE::CONFIDENCE` (0–100)
   - Optional left RGB image for overlay
3. Mask Construction:
   - Valid if: inside `[minDepth,maxDepth]`, finite, >0, and confidence <= threshold (ZED: 0=best, 100=worst)
   - Fallback: if too few pixels remain after confidence filtering, ignore the confidence mask for that frame to avoid fully black outputs
4. Adaptive Range (when `autoContrast`):
   - Collect valid depths; compute 2nd and 98th percentiles (fallback to configured min/max if span < 0.5 m)
5. Scaling:
   - Linear scaling of (depth - a)/(b - a), clamped, inverted (near = high value)
   - Invalid pixels forced to 0
6. Colormap:
   - Selectable: Turbo (default), Viridis, Plasma, Jet
7. Optional processing:
   - Edge Emphasis: Sobel gradient magnitude boosts thin structures
   - CLAHE: local contrast on 8-bit scaled depth
   - Temporal Smoothing (EMA): reduce flicker; alpha configurable
   - Log Scaling: alternative normalization for wide dynamic ranges
8. Overlay:
   - `addWeighted(heatmap, alpha, leftRGB, 1-alpha)` if enabled
8. Output:
   - PNG heatmap (or blended overlay)
   - Optional AVI video (MJPEG) using blended or plain heatmap frames
   - Metadata records overlay + transparency and adaptive config

## Rationale
- Percentile scaling increases effective contrast for the active depth distribution (reduces dominance of far horizon)
- Confidence mask removes unstable regions causing speckle and false structure that can hide thin objects
- Turbo colormap offers improved perceptual uniformity vs JET
- Overlay uses native resolution, preserving fine details

## Planned Enhancements (Backlog)
| Priority | Feature | Description | Notes |
|----------|---------|-------------|-------|
| Medium | Motion Emphasis | Mix |depth-EMA| to highlight movers | After temporal smoothing |
| Medium | Recorded Preview Panel | Show last N generated frames as thumbnails in GUI | Requires OpenGL texture upload |
| Low | Edge-Only Overlay Mode | Render RGB where edges exist, else heatmap; or vice versa | Experimental |
| Low | Object Band Isolation | Auto-detect secondary smaller depth cluster and apply distinct color | Histogram-based clustering |
| Low | 16-bit Intermediate Saves | Save a 16-bit normalized depth PNG alongside heatmap for analysis | Avoids EXR dependency for some workflows |

## Proposed API Additions
`DepthExtractionConfig` future fields:
- `bool useClahe` (default false)
- `bool useEdgeBoost` (default false)
- `float edgeBoostFactor` (0–2)
- `bool useTemporalSmooth` (default false)
- `float temporalAlpha` (0.1–0.5)
- `std::string colorMap` ("turbo","viridis","plasma","jet")
- `bool logScale` (mutually exclusive with autoContrast)

## Edge Boost Sketch
```cpp
// After scaled (0..1)
cv::Mat gx, gy, grad;
cv::Sobel(depthFloat, gx, CV_32F, 1, 0, 3);
cv::Sobel(depthFloat, gy, CV_32F, 0, 1, 3);
cv::magnitude(gx, gy, grad);
cv::Mat gradMask; cv::normalize(grad, gradMask, 0, 1, cv::NORM_MINMAX);
scaled = cv::min(1.0f, scaled + edgeBoostFactor * gradMask);
```

## Temporal Smoothing Sketch
```cpp
// Maintain static cv::Mat ema (float)
if (ema.empty()) ema = depthFloat.clone();
ema = temporalAlpha*depthFloat + (1.0f-temporalAlpha)*ema;
cv::Mat motion = cv::abs(depthFloat - ema); // could weight edge boost
```

## Testing Checklist
- Verify airplane retention across frames (visual inspection)
- Compare with and without confidence mask (e.g., pixel count of valid mask)
- Measure histogram spread before/after percentile scaling
- FPS impact: log time per processed frame with optional features toggled

## Performance Considerations
- Percentile computation currently extracts values into vector each frame (O(N)); can optimize by downsampling grid (e.g. stride 2) or using approximate quantiles.
- Edge and bilateral filtering add ~1–3 ms depending on resolution/GPU; test before enabling by default.
- Temporal smoothing requires state; thread-safety needed if multi-threading future extractions.

## Open Questions
- Do we need right-eye depth overlay? (Enable later via `enable_right_side_measure` if required.)
- Should adaptive range persist across frames (sliding window) to avoid flicker between percentile shifts? (Windowed histogram smoothing.)

## Next Implementation Steps
1. Add CLAHE + Viridis colormap selector
2. Edge boost toggle
3. Temporal smoothing
4. Preview thumbnails
5. Log scaling mode

This document should be updated as features move from “Planned” to “Implemented.”
