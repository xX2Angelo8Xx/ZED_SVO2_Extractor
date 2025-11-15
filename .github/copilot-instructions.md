# AI Agent Quickstart – ZED_SVO2_Extractor

Concise, action‑oriented guidance for coding agents. Cite concrete files, prefer focused diffs over prose. Stay project‑specific (avoid generic advice).

## Architecture & Flow
Apps: CLI video (`apps/video_extractor/`), CLI frames (`apps/frame_extractor/`), GUI (`apps/gui_extractor/`). Shared library: `common/` (core: `svo_handler.*`, `output_manager.*`, `file_utils.*`, `metadata.*`, `error_handler.*`, `extraction_engine.*`). Typical pipeline: parse args → validate SVO2 (`FileUtils::validateSVO2File`) → open via `SVOHandler` → derive flight folder & output (`OutputManager`) → iterate frames (ZED SDK) → convert BGRA→BGR → encode/write → record metadata JSON → log (`LOG_INFO/...`). GUI adds threading + cancel via `ExtractionEngine` atomics.

## Build (Windows)
Deps: ZED SDK 4.x (`ZED_SDK_ROOT_DIR`), external OpenCV 4.x at `C:/opencv` for H.264/H.265 (else bundled 3.1.0 → MJPEG only), optional CUDA (depth + future features). Generate from `build/`:
`cmake .. -G "Visual Studio 17 2022" -A x64` then `cmake --build . --config Release --target gui_extractor video_extractor_cli frame_extractor_cli`. Executables land in `build/bin/Release/`.

## Core Conventions
Flight folder pattern: `flight_YYYYMMDD_HHMMSS` (check with `FileUtils::isFlightFolder`). Video/Depth output: `<base>/Extractions/<flight>/extraction_NNN/`. Frame (YOLO) output: `<base>/Yolo_Training/Unfiltered_Images/<flight>/` with global 6‑digit counter (`OutputManager::getNextGlobalFrameNumber`, then `updateGlobalFrameCounter`). Metadata path: `OutputManager::getMetadataPath(extractionPath)`.
Frame names: `L_frame_000123.png` / `R_frame_000123.png` (left/right). Side‑by‑side mode composes stereo into one frame before encode.

## ExtractionEngine Patterns (`common/extraction_engine.*`)
APIs: `extractFrames`, `extractVideo`, `extractDepth(DepthExtractionConfig, ProgressCallback)`. Progress callback signature: `(float 0..1, std::string status)`. Cancel via `cancel()`; internal flags: `cancelRequested_`, `isRunning_`. Depth live preview retrieval: `getLatestDepthPreview`, legend/info via `getLatestDepthPreviewInfo` / `getLatestDepthLegend` (mutex protected).
Add feature: extend relevant *Config struct + implement logic + surface in GUI (`apps/gui_extractor/gui_application.*`) & CLI (argument parsing). Keep names lower_snake for config fields.

## Codecs & Media
Default video codec: MJPEG (`cv::VideoWriter::fourcc('M','J','P','G')`) for portability. H.264/H.265 only when external OpenCV with FFmpeg support detected. Fallback: if requested output FPS > source, engine/CLI should clamp to source (see CLI validation). Always convert ZED BGRA frame to BGR before writing.

## Logging & Errors
Initialize logger once (e.g., early in `main`). Use `LOG_INFO/LOG_WARNING/LOG_ERROR` macros; `LOG_DEBUG` compiled only with `_DEBUG`. For operations returning `ErrorResult` (e.g., config validation) check `isSuccess()` and log failure immediately.

## Depth Extraction Specifics
Config highlights: `depthMode` (e.g., NEURAL / NEURAL_PLUS), `minDepth` & `maxDepth` for color scaling, optional enhancements (`useClahe`, `useTemporalSmooth`, `highlightMotion`). Overlay blends heatmap onto RGB controlled by `overlayStrength`. Color maps: turbo | viridis | plasma | jet. Motion emphasis uses frame‑to‑frame depth diff (`motionGain`).

## Adding / Modifying Output Rules
Edit `output_manager.*`: global frame numbering scan + extraction folder increment logic (`getNextExtractionNumber`). Ensure new folder patterns continue to be deterministic and incremental. When changing, update README + examples.

## Exemplars
CLI argument parsing + validation: `apps/video_extractor/video_extractor_cli.cpp` & `apps/frame_extractor/frame_extractor_cli.cpp`.
Codec selection + BGRA conversion: same video CLI file (`slMat2cvMat`, `getVideoCodec`).
Threaded progress/cancel: `extraction_engine.cpp` (look at report/cancel helpers).
Flight detection & size formatting: `file_utils.*`.
Global numbering & metadata pathing: `output_manager.cpp`.

## Common Gotchas
Missing external OpenCV → H.264/H.265 fail silently: force MJPEG. Ensure DLL copy (OpenCV + ZED) into runtime dir (`build/bin/Release/`). OneDrive path length & spaces: sanitize via `FileUtils::sanitizeFilename`. Depth preview race: lock with provided mutex functions (don’t add ad‑hoc locks). Avoid blocking UI thread: run engine calls on worker thread and poll `getLatestDepthPreview`.

## Minimal Example (Video)
```cpp
using zed_extractor::ExtractionEngine;
using zed_extractor::VideoExtractionConfig;

void onProgress(float p, const std::string& msg) {
	LOG_INFO("Progress " << int(p * 100) << "% - " << msg);
}

ExtractionEngine engine;
VideoExtractionConfig cfg{ /*svoFilePath=*/svo,
						   /*baseOutputPath=*/base,
						   /*cameraMode=*/"side_by_side",
						   /*codec=*/"mjpeg",
						   /*outputFps=*/0.0f,  // 0 = source FPS
						   /*quality=*/95 };
auto res = engine.extractVideo(cfg, onProgress);
if (!res.success) LOG_ERROR(res.errorMessage);
```

Keep diffs narrow: change feature code + update docs if user‑visible. Preserve naming/style (see `docs/CODE_STYLE_GUIDE.md`). Ask for confirmation before large refactors.