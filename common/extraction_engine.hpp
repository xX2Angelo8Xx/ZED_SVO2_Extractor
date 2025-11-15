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
#include <mutex>
#include <opencv2/core.hpp>

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
 * @brief Depth extraction configuration
 */
struct DepthExtractionConfig {
    std::string svoFilePath;
    std::string baseOutputPath;
    float outputFps = 1.0f;           // FPS for depth map extraction (1-30)
    float minDepth = 10.0f;           // Minimum depth in meters (for colorization)
    float maxDepth = 40.0f;           // Maximum depth in meters (for colorization)
    bool saveRawDepth = false;        // Save raw 32-bit float depth values
    // Raw depth format preference. "exr" will attempt EXR (if OpenEXR enabled),
    // "tiff32f" forces 32-bit float TIFF, "pfm" writes Portable Float Map,
    // "bin" writes a simple binary dump (width*height float32 little-endian).
    // Default uses tiff32f for broad compatibility.
    std::string rawDepthFormat = "tiff32f";
    bool saveColorized = true;        // Save colorized heatmap (PNG)
    bool saveVideo = false;           // Create video from depth maps
    bool saveRgbFrames = false;       // Save left RGB frames for fast re-render overlay
    bool saveConfidenceMaps = false;  // Save confidence maps (8-bit) for debugging/masking
    std::string depthMode = "NEURAL"; // PERFORMANCE, QUALITY, ULTRA, NEURAL, NEURAL_PLUS
    bool overlayOnRgb = true;         // Blend heatmap over left RGB image
    int overlayStrength = 100;        // 0 = only RGB, 100 = only heatmap
    bool autoContrast = true;         // Use percentile-based contrast stretching per frame
    int confidenceThreshold = 60;     // 0-100, low values allow more pixels; high values remove noisy pixels
    bool useEdgeBoost = false;        // Apply edge (gradient) boost
    float edgeBoostFactor = 0.7f;     // Multiplier for edge enhancement (0-2)
    bool useClahe = false;            // Apply CLAHE local contrast
    bool useTemporalSmooth = false;   // Enable temporal EMA smoothing
    float temporalAlpha = 0.3f;       // EMA alpha (0.1-0.5 typical)
    bool logScale = false;            // Use logarithmic scaling instead of linear
    std::string colorMap = "turbo";   // turbo, viridis, plasma, jet
    bool highlightMotion = false;     // Emphasize moving objects via depth difference
    float motionGain = 0.6f;          // Strength of motion highlight (0-1)
    bool storePreviews = true;        // Keep per-frame preview images for navigation
    int previewMaxWidth = 960;        // Downscale previews to this width (preserve aspect); <=0 = no downscale
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
     * @brief Extract depth maps from SVO file
     * @param config Depth extraction configuration
     * @param progressCallback Optional progress callback
     * @return Extraction result
     */
    ExtractionResult extractDepth(
        const DepthExtractionConfig& config,
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

    /**
     * @brief Retrieve latest preview image (heatmap or overlay) produced during depth extraction.
     * @param out Destination cv::Mat (BGR) clone of latest preview.
     * @param version Preview version counter.
     * @return true if a preview is available.
     */
    bool getLatestDepthPreview(cv::Mat& out, int& version) const;
    // Retrieve latest raw float depth (CV_32FC1) if available.
    bool getLatestRawDepth(cv::Mat& out) const;

    struct DepthPreviewInfo {
        double minMeters = 0.0;
        double maxMeters = 0.0;
        bool autoContrast = false;
        bool logScale = false;
        int confidenceThreshold = 0;
        bool overlayOnRgb = false;
        int overlayStrength = 0; // 0..100
        std::string colorMap;     // turbo/viridis/plasma/jet
    };

    /**
     * @brief Retrieve latest legend/scale info for the depth preview.
     * @param out Filled with the latest info.
     * @param version Version counter aligned with preview image updates.
     * @return true if info is available.
     */
    bool getLatestDepthPreviewInfo(DepthPreviewInfo& out, int& version) const;
    bool getLatestDepthLegend(cv::Mat& out, int& version) const;

    // Stored previews API (post-run browsing)
    int getStoredPreviewCount() const;
    bool getStoredPreviewAt(int index, cv::Mat& out) const;
    bool setStoredPreviewAt(int index, const cv::Mat& img);
    int getStoredFrameIndexAt(int index) const; // original SVO frame index

    // Single-frame re-render using current or new parameters.
    // If overwriteSaved is true and a prior heatmap exists, it will be overwritten.
    bool reprocessDepthFrame(int storedIndex,
                             const DepthExtractionConfig& cfg,
                             cv::Mat& outPreview,
                             bool overwriteSaved = true);
    // Fetch raw float depth for a stored frame index by re-seeking the SVO
    bool getDepthFloatForStored(int storedIndex,
                                const DepthExtractionConfig& cfg,
                                cv::Mat& outDepthFloat);
    // Load saved confidence map (8-bit) for a stored frame if available
    bool getConfidenceForStored(int storedIndex, cv::Mat& outConf8u) const;
    // Load saved left RGB frame (BGR8) for a stored frame if available
    bool getRgbForStored(int storedIndex, cv::Mat& outBgr) const;

private:
    std::atomic<bool> cancelRequested_;
    std::atomic<bool> isRunning_;
    mutable std::mutex previewMutex_;
    cv::Mat latestPreview_;
    cv::Mat latestRawDepth_; // CV_32FC1
    std::atomic<int> previewVersion_{0};
    DepthPreviewInfo latestPreviewInfo_;
    cv::Mat latestLegend_;
    // Stored previews for navigation
    std::vector<cv::Mat> storedPreviews_;     // BGR8, possibly downscaled
    std::vector<int> storedFrameIndices_;     // Original frame indices in SVO
    std::string lastExtractionPath_;          // Path of the last depth extraction output
    
    // Internal helper to check cancellation
    bool shouldCancel() const;
    
    // Internal helper to report progress
    void reportProgress(float progress, const std::string& message, ProgressCallback callback);
};

} // namespace zed_extractor
