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
    
    // Frame extractor settings
    float frameFps_;
    int frameCamera_;
    int frameFormat_;
    
    // Video extractor settings
    int videoCamera_;
    int videoCodec_;
    float videoFps_;
    int videoQuality_;
    
    // Progress tracking
    std::atomic<bool> isProcessing_;
    std::atomic<float> progressValue_;
    std::string progressMessage_;
    std::mutex progressMutex_;
    
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
    
    // ImGui helpers
    void setupStyle();
    void processEvents();
    void beginFrame();
    void endFrame();
};

} // namespace zed_gui

