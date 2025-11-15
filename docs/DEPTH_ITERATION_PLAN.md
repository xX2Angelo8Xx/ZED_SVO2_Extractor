# Depth Iteration & Thin Airplane Detection

Last updated: 2025-11-10

This document tracks the GUI/engine work to improve depth-based detection of small, thin model airplanes and to speed up parameter iteration.

## Goals
- Fix GUI bug: depth preview shows first frame then stops.
- Keep 32-bit depth fidelity available for re-rendering.
- Show every computed frame in the GUI while extracting.
- After extraction, allow navigating frames and re-rendering a single frame with new parameters.
- Focus parameters on ZED SDK RuntimeParameters & depth settings.

References:
- RuntimeParameters: https://www.stereolabs.com/docs/api/structsl_1_1RuntimeParameters.html
- Depth settings: https://www.stereolabs.com/docs/depth-sensing/depth-settings
- Using depth: https://www.stereolabs.com/docs/depth-sensing/using-depth

## Current Status
- Fixed unsafe ImGui InputText usage that could corrupt memory and stall preview updates.
- Continuous live preview pipeline exists in `common/extraction_engine.cpp` and `apps/gui_extractor/gui_application.cpp`.

## Work Plan (Phased)

### Phase 1 – Stabilization (GUI bugfix)
- [x] Replace unsafe `ImGui::InputText` on std::string with fixed buffers.
- [x] Sync edited output path buffer to `outputPath_` before starting extraction.
- [ ] Re-test: Ensure `previewVersion_` increments and GUI updates per frame.
- [ ] Optional: Add DEBUG logs on preview updates when `_DEBUG`.

### Phase 2 – Core Data Capture & Navigation
- [ ] Config: Add `captureAllDepthRaw` and `storePreviews` to `DepthExtractionConfig`.
- [ ] Engine: Store previews (downscaled if needed) in-memory; optionally save EXR per frame if `captureAllDepthRaw`.
- [ ] GUI: Add post-run navigation (Prev/Next, index display) to browse stored frames.

### Phase 3 – Single-Frame Re-render
- [ ] Engine: `reprocessDepthFrame(index, newCfg)` that uses EXR or seeks SVO to recompute one frame.
- [ ] GUI: "Re-render" button applies current UI parameters to current frame; replace preview and (optionally) overwrite saved heatmap.

### Phase 4 – Parameter Focus & Presets
- [ ] Simplify primary controls to ZED-aligned params: `depth_mode`, `confidence_threshold`, `minDepth`, `maxDepth`.
- [ ] Move motion/edge/CLAHE/temporal/log to an Advanced accordion (default collapsed).
- [ ] Add presets: Baseline / High Confidence / Wide Range / Fine Detail.

### Phase 5 – Debugging Aids
- [ ] Depth histogram per current frame to guide min/max.
- [ ] ROI zoom/magnifier to inspect thin objects.
- [ ] Optional edges overlay (Canny) to help visualize small silhouettes.

### Phase 6 – Automation
- [ ] CLI "depth_probe" to dump per-frame stats (valid pixel counts in ranges, medians). 
- [ ] PowerShell `scripts/depth_param_sweep.ps1` to iterate parameter grids, saving metrics to CSV.

## Design Notes
- Memory: 1080p float depth ~8MB/frame; full capture over thousands of frames is large. Prefer:
  - In-memory: downscaled previews (e.g., 960x540, BGR8).
  - On-disk: EXR per frame only if interactive re-render needed.
  - On-demand seek: If EXR disabled, seek SVO to frame for re-render.
- Fidelity: Keep BGRA→BGR for previews; retain float depth as source for rendering.
- Threading: Engine owns extraction thread; GUI pulls preview via versioned getters.

## Acceptance Criteria
- Live preview progresses continuously for entire SVO in depth mode.
- After run, arrow keys/buttons navigate rendered frames instantly.
- Re-rendering a single frame with new parameters updates only that frame and is visible within 1–2s on typical hardware.
- Default parameters emphasize thin-object visibility without excessive noise.

## File Touchpoints
- GUI: `apps/gui_extractor/gui_application.*`
- Engine: `common/extraction_engine.*`
- Utils: `common/file_utils.*`, `common/output_manager.*`
- Docs: this file, `docs/DEPTH_VISUALIZATION.md` (update screenshots when UX lands)
