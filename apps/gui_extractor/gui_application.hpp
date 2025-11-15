/**
 * @file gui_application.hpp
 * @brief GUI Application for ZED SVO2 Extractor
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include "../../common/extraction_engine.hpp"
#include <opencv2/core.hpp>
// ImGui types used in header (ImVec2)
#include <imgui.h>

// Forward declarations
struct GLFWwindow;

namespace zed_gui {

/**
 * @brief Main GUI Application class
 * 
 * Manages the Dear ImGui application lifecycle, window creation,
 * and provides interface for frame/video/depth extraction tools.
 */
class GUIApplication {
public:
    GUIApplication();
    ~GUIApplication();

    /**
     * @brief Initialize the application
     * @param width Initial window width
     * @param height Initial window height
     * @param title Window title
     * @return true if successful
     */
    bool initialize(int width = 1280, int height = 720, const char* title = "ZED SVO2 Extractor");

    /**
     * @brief Run the main application loop
     */
    void run();

    /**
     * @brief Shutdown and cleanup
     */
    void shutdown();

private:
    // Window management
    GLFWwindow* window_;
    bool initialized_;

    // GUI state
    int currentTab_;
    std::string svoFilePath_;
    std::string outputPath_;
    // Safe UI buffers for ImGui text inputs
    char svoPathBuf_[512]{};
    char outPathBuf_[512]{};
    
    // Frame extractor settings
    float frameFps_;
    int frameCamera_;
    int frameFormat_;
    
    // Video extractor settings
    int videoCamera_;
    int videoCodec_;
    float videoFps_;
    int videoQuality_;
    
    // Depth extractor settings
    int depthMode_;            // 0: NEURAL, 1: NEURAL_PLUS, 2: PERFORMANCE, 3: QUALITY, 4: ULTRA
    float depthOutputFps_;     // 1-30
    float depthMinMeters_;     // e.g., 0.5 - 50m
    float depthMaxMeters_;     // e.g., 1 - 100m
    bool depthSaveRaw_;        // Save EXR
    int  depthRawFormatIndex_; // 0: TIFF 32F, 1: PFM, 2: EXR, 3: BIN
    bool depthSaveColorized_;  // Save PNG heatmaps
    bool depthSaveVideo_;      // Create AVI from heatmaps
    bool depthOverlayEnabled_; // Blend heatmap over RGB
    int  depthOverlayStrength_; // 0..100 (% heatmap)
    bool depthSaveConfidence_ = false; // Save confidence maps for viewer
    bool depthSaveRgbFrames_ = false; // Save left RGB frames for fast re-render
    bool depthAutoContrast_;    // Percentile scaling enable
    int  depthConfidenceThresh_; // Confidence threshold 0-100
    bool depthEdgeBoost_;        // Edge emphasis
    float depthEdgeFactor_;      // Edge boost factor
    bool depthClahe_;            // CLAHE enable
    bool depthTemporal_;         // Temporal smoothing
    float depthTemporalAlpha_;   // EMA alpha
    bool depthLogScale_;         // Log scaling
    int depthColorMapIndex_;     // Selected colormap
    bool depthHighlightMotion_;  // Motion emphasis
    float depthMotionGain_;      // Motion highlight strength
    
    // Progress tracking
    std::atomic<bool> isProcessing_;
    std::atomic<float> progressValue_;
    std::string progressMessage_;
    std::mutex progressMutex_;

    // Live depth preview
    cv::Mat depthPreview_;              // latest preview image (BGR/RGB)
    int depthPreviewVersion_;           // version counter from engine
    unsigned int depthPreviewTexture_;  // OpenGL texture id
    int depthPreviewWidth_;
    int depthPreviewHeight_;
    std::mutex depthPreviewMutex_;      // guards preview updates

    // Navigation state (post-run frame browsing)
    int navIndex_ = -1;                 // -1 = live latest; >=0 = stored preview index
    int navCount_ = 0;
    int navStep_ = 1;                   // step size for navigation (1 or 5)

    // Legend rendering
    unsigned int legendTexture_ = 0;    // OpenGL texture for colorbar
    int legendWidth_ = 256;
    int legendHeight_ = 16;
    int legendVersionSeen_ = -1;        // last version we synced from engine
    std::string legendColorMap_ = "turbo";
    double legendMinMeters_ = 0.0;
    double legendMaxMeters_ = 0.0;
    bool legendAutoContrast_ = false;
    bool legendLogScale_ = false;
    int legendConfidence_ = 0;
    
    // Extraction engine and threading
    std::unique_ptr<zed_extractor::ExtractionEngine> engine_;
    std::unique_ptr<std::thread> extractionThread_;
    
    // Result storage
    std::string lastResultMessage_;
    bool lastResultSuccess_;

    // Private methods
    void renderMenuBar();
    void renderMainContent();
    void renderFrameExtractorTab();
    void renderVideoExtractorTab();
    void renderDepthExtractorTab();
    void renderStatusBar();
    
    void selectSVOFile();
    void selectOutputPath();
    
    void startFrameExtraction();
    void startVideoExtraction();
    void startDepthExtraction();
    
    void cancelExtraction();
    void checkExtractionComplete();
    
    void updateProgress(float progress, const std::string& message);

    // Depth preview helpers
    void updateDepthPreview();
    void renderDepthPreviewPane();
    void uploadLegendTexture(const cv::Mat& legendBgr);
    void renderDepthNavigator();
    void triggerRerenderSelected();
    void renderRawDepthWindow();
    bool showRawDepthWindow_ = false;
    unsigned int rawDepthTexture_ = 0;
    int rawDepthWidth_ = 0;
    int rawDepthHeight_ = 0;
    // Cache for stored raw depth to avoid repeated disk I/O each frame
    int rawCacheIndex_ = -2; // -2 = uninitialized, -1 = live latest, >=0 = stored index
    cv::Mat rawCache_;
    float rawViewerMin_ = 0.0f;
    float rawViewerMax_ = 0.0f;
    bool rawViewerAutoApply_ = true;
    bool rawViewerUseConfMask_ = false;
    int  rawViewerConfThresh_ = 60; // 0..100
    bool rawViewerUseLog_ = false;
    bool rawViewerAutoContrast_ = false;
    bool rawViewerRequestFocus_ = false;
    // cache conf map parallel to raw depth cache
    int confCacheIndex_ = -2;
    cv::Mat confCache8_;
    // optional RGB overlay cache
    bool rawViewerOverlayRgb_ = false;
    int  rawViewerOverlayStrength_ = 50; // 0..100
    int  rgbCacheIndex_ = -2;
    cv::Mat rgbCacheBgr_;
    // picking / ROI
    bool rawSelecting_ = false;
    ImVec2 rawSelStart_ = ImVec2(0,0);
    ImVec2 rawSelEnd_ = ImVec2(0,0);
    // zoom/pan state for raw viewer
    float rawZoom_ = 1.0f;    // 1.0 = fit canvas
    ImVec2 rawPan_ = ImVec2(0,0); // screen-space pan offset within canvas
    int roiX1_ = 0, roiY1_ = 0, roiX2_ = 0, roiY2_ = 0;
    float roiAvg_ = 0.0f, roiMin_ = 0.0f, roiMax_ = 0.0f; int roiCount_ = 0;
    float lastPickDepth_ = 0.0f; int lastPickX_ = -1, lastPickY_ = -1;
    
    // ImGui helpers
    void setupStyle();
    void processEvents();
    void beginFrame();
    void endFrame();
};

} // namespace zed_gui

