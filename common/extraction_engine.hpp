/**
 * @file extraction_engine.hpp
 * @brief Unified extraction engine for frame and video extraction with progress callbacks
 * @author ZED SVO2 Extractor Team
 * @date 2025-11-07
 */

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>

namespace zed_extractor {

/**
 * @brief Progress callback signature
 * @param progress Progress value (0.0 to 1.0)
 * @param message Status message
 */
using ProgressCallback = std::function<void(float progress, const std::string& message)>;

/**
 * @brief Frame extraction configuration
 */
struct FrameExtractionConfig {
    std::string svoFilePath;
    std::string baseOutputPath;
    float fps = 1.0f;
    std::string cameraMode = "left";  // left, right, both
    std::string format = "png";       // png, jpg
};

/**
 * @brief Video extraction configuration
 */
struct VideoExtractionConfig {
    std::string svoFilePath;
    std::string baseOutputPath;
    std::string cameraMode = "left";  // left, right, both_separate, side_by_side
    std::string codec = "h264";       // h264, h265, mjpeg
    float outputFps = 0.0f;           // 0 = use source FPS
    int quality = 100;                // 50-100
};

/**
 * @brief Extraction result
 */
struct ExtractionResult {
    bool success = false;
    std::string errorMessage;
    std::string outputPath;
    int framesProcessed = 0;
    
    static ExtractionResult Success(const std::string& path, int frames = 0) {
        ExtractionResult result;
        result.success = true;
        result.outputPath = path;
        result.framesProcessed = frames;
        return result;
    }
    
    static ExtractionResult Failure(const std::string& error) {
        ExtractionResult result;
        result.success = false;
        result.errorMessage = error;
        return result;
    }
};

/**
 * @brief Main extraction engine class
 * Thread-safe extraction with progress callbacks and cancellation support
 */
class ExtractionEngine {
public:
    ExtractionEngine();
    ~ExtractionEngine();
    
    /**
     * @brief Extract frames from SVO file
     * @param config Frame extraction configuration
     * @param progressCallback Optional progress callback
     * @return Extraction result
     */
    ExtractionResult extractFrames(
        const FrameExtractionConfig& config,
        ProgressCallback progressCallback = nullptr
    );
    
    /**
     * @brief Extract video from SVO file
     * @param config Video extraction configuration
     * @param progressCallback Optional progress callback
     * @return Extraction result
     */
    ExtractionResult extractVideo(
        const VideoExtractionConfig& config,
        ProgressCallback progressCallback = nullptr
    );
    
    /**
     * @brief Cancel ongoing extraction
     */
    void cancel();
    
    /**
     * @brief Check if extraction is in progress
     */
    bool isRunning() const;

private:
    std::atomic<bool> cancelRequested_;
    std::atomic<bool> isRunning_;
    
    // Internal helper to check cancellation
    bool shouldCancel() const;
    
    // Internal helper to report progress
    void reportProgress(float progress, const std::string& message, ProgressCallback callback);
};

} // namespace zed_extractor
