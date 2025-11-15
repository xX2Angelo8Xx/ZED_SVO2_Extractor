/**
 * @file gui_application.cpp
 * @brief Implementation of GUI Application
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

// Ensure Windows.h doesn't define min/max macros that break std::min/std::max and OpenCV headers
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "gui_application.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <opencv2/imgproc.hpp>

// Fallback GL constants if loader didn't define them (needed before first use)
#ifndef GL_RGB
#define GL_RGB 0x1907
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <commdlg.h>  // For file dialogs
#include <shlobj.h>   // For folder dialogs
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
// Just in case, drop any lingering macros
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

namespace zed_gui {

GUIApplication::GUIApplication()
    : window_(nullptr)
    , initialized_(false)
    , currentTab_(0)
    , svoFilePath_("")
    , outputPath_("E:/Turbulence Solutions/AeroLock/ZED_Recordings_Output")
    , frameFps_(1.0f)
    , frameCamera_(0)
    , frameFormat_(0)
    , videoCamera_(0)
    , videoCodec_(0)
    , videoFps_(0.0f)
    , videoQuality_(100)  // Default to maximum quality
    , depthMode_(0)
    , depthOutputFps_(5.0f)
    , depthMinMeters_(10.0f)
    , depthMaxMeters_(40.0f)
    , depthSaveRaw_(false)
    , depthSaveColorized_(true)
    , depthSaveVideo_(false)
    , depthOverlayEnabled_(true)
    , depthOverlayStrength_(100)
    , depthAutoContrast_(true)
    , depthConfidenceThresh_(60)
    , depthEdgeBoost_(false)
    , depthEdgeFactor_(0.7f)
    , depthClahe_(false)
    , depthTemporal_(false)
    , depthTemporalAlpha_(0.3f)
    , depthLogScale_(false)
    , depthColorMapIndex_(0) // 0: Turbo, 1: Viridis, 2: Plasma, 3: Jet
    , depthHighlightMotion_(false)
    , depthMotionGain_(0.6f)
    , isProcessing_(false)
    , progressValue_(0.0f)
    , progressMessage_("")
    , engine_(nullptr)
    , extractionThread_(nullptr)
    , lastResultMessage_("")
    , lastResultSuccess_(false)
{
    engine_ = std::make_unique<zed_extractor::ExtractionEngine>();
    depthPreviewVersion_ = -1;
    depthPreviewTexture_ = 0;
    depthPreviewWidth_ = 0;
    depthPreviewHeight_ = 0;
    // Initialize safe text buffers
    memset(svoPathBuf_, 0, sizeof(svoPathBuf_));
    memset(outPathBuf_, 0, sizeof(outPathBuf_));
    strncpy(outPathBuf_, outputPath_.c_str(), sizeof(outPathBuf_) - 1);
}

GUIApplication::~GUIApplication() {
    if (extractionThread_ && extractionThread_->joinable()) {
        engine_->cancel();
        extractionThread_->join();
    }
    if (depthPreviewTexture_ != 0) {
        glDeleteTextures(1, &depthPreviewTexture_);
        depthPreviewTexture_ = 0;
    }
    if (legendTexture_ != 0) {
        glDeleteTextures(1, &legendTexture_);
        legendTexture_ = 0;
    }
    if (initialized_) {
        shutdown();
    }
}

bool GUIApplication::initialize(int width, int height, const char* title) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Requires docking branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    setupStyle();

    initialized_ = true;
    std::cout << "GUI Application initialized successfully" << std::endl;
    return true;
}

void GUIApplication::run() {
    if (!initialized_) {
        std::cerr << "Application not initialized" << std::endl;
        return;
    }

    // Main loop
    while (!glfwWindowShouldClose(window_)) {
        processEvents();
        beginFrame();
        
        // Check if extraction completed
        checkExtractionComplete();
        
    // Update any live previews from engine
    updateDepthPreview();

    // Render GUI
        renderMenuBar();
        renderMainContent();
        renderStatusBar();
        
        endFrame();
    }
}

void GUIApplication::shutdown() {
    if (!initialized_) return;

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup GLFW
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();

    initialized_ = false;
    std::cout << "GUI Application shut down" << std::endl;
}

void GUIApplication::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Color scheme - Modern dark theme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.40f, 0.60f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.40f, 0.60f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.40f, 0.60f, 1.00f);
    
    // Rounding
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
}

void GUIApplication::processEvents() {
    glfwPollEvents();
}

void GUIApplication::beginFrame() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUIApplication::endFrame() {
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window_);
}

void GUIApplication::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open SVO...", "Ctrl+O")) {
                selectSVOFile();
            }
            if (ImGui::MenuItem("Set Output Path..")) {
                selectOutputPath();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(window_, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Raw Depth Viewer", nullptr, &showRawDepthWindow_)) {
                if (showRawDepthWindow_) {
                    rawViewerRequestFocus_ = true; rawCacheIndex_ = -2; rawCache_.release(); confCacheIndex_ = -2; confCache8_.release();
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // TODO: Show about dialog
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void GUIApplication::renderMainContent() {
    // Full window content
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 
                                     ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight() * 2));
    
    ImGui::Begin("Main", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // File selection
    ImGui::Text("SVO File:");
    ImGui::SameLine();
    // Display read-only path safely
    ImGui::TextWrapped("%s", svoFilePath_.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Browse##svo")) {
        selectSVOFile();
    }

    ImGui::Text("Output Path:");
    ImGui::SameLine();
    // Editable text buffer (kept in outPathBuf_). We'll sync to outputPath_ on use.
    ImGui::InputText("##outpath", outPathBuf_, sizeof(outPathBuf_));
    ImGui::SameLine();
    if (ImGui::Button("Browse##out")) {
        selectOutputPath();
    }

    ImGui::Separator();

    // Tabs for different extraction modes
    if (ImGui::BeginTabBar("ExtractionModes")) {
        if (ImGui::BeginTabItem("Frame Extraction")) {
            renderFrameExtractorTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Video Extraction")) {
            renderVideoExtractorTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Depth Extraction")) {
            renderDepthExtractorTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void GUIApplication::renderFrameExtractorTab() {
    ImGui::Text("Extract frames for YOLO training");
    ImGui::Separator();

    ImGui::SliderFloat("FPS", &frameFps_, 0.1f, 30.0f, "%.1f");
    
    const char* cameras[] = { "Left", "Right", "Both" };
    ImGui::Combo("Camera", &frameCamera_, cameras, IM_ARRAYSIZE(cameras));
    
    const char* formats[] = { "PNG", "JPG" };
    ImGui::Combo("Format", &frameFormat_, formats, IM_ARRAYSIZE(formats));

    ImGui::Separator();
    
    if (isProcessing_) {
        std::lock_guard<std::mutex> lock(progressMutex_);
        ImGui::ProgressBar(progressValue_, ImVec2(-1, 0), progressMessage_.c_str());
        if (ImGui::Button("Cancel", ImVec2(-1, 30))) {
            cancelExtraction();
        }
    } else {
        if (ImGui::Button("Start Frame Extraction", ImVec2(-1, 40))) {
            startFrameExtraction();
        }
    }
}

void GUIApplication::renderVideoExtractorTab() {
    ImGui::Text("Extract video from SVO file");
    ImGui::Separator();

    const char* cameras[] = { "Left", "Right", "Both Separate", "Side-by-Side" };
    ImGui::Combo("Camera Mode", &videoCamera_, cameras, IM_ARRAYSIZE(cameras));
    
    const char* codecs[] = { "H264", "H265", "MJPEG" };
    ImGui::Combo("Codec", &videoCodec_, codecs, IM_ARRAYSIZE(codecs));
    
    ImGui::SliderFloat("FPS (0=source)", &videoFps_, 0.0f, 100.0f, "%.0f");
    ImGui::SliderInt("Quality", &videoQuality_, 50, 100, "%d%%");

    ImGui::Separator();
    
    if (isProcessing_) {
        std::lock_guard<std::mutex> lock(progressMutex_);
        ImGui::ProgressBar(progressValue_, ImVec2(-1, 0), progressMessage_.c_str());
        if (ImGui::Button("Cancel", ImVec2(-1, 30))) {
            cancelExtraction();
        }
    } else {
        if (ImGui::Button("Start Video Extraction", ImVec2(-1, 40))) {
            startVideoExtraction();
        }
    }
}

void GUIApplication::renderDepthExtractorTab() {
    ImGui::Text("Depth map extraction and heatmap video");
    ImGui::Separator();

    // Live preview pane
    renderDepthPreviewPane();
    renderDepthNavigator();

    const char* modes[] = { "NEURAL", "NEURAL_PLUS", "PERFORMANCE", "QUALITY", "ULTRA" };
    ImGui::Combo("Depth Mode", &depthMode_, modes, IM_ARRAYSIZE(modes));

    ImGui::SliderFloat("Output FPS", &depthOutputFps_, 1.0f, 30.0f, "%.0f");
    ImGui::SliderFloat("Min Depth (m)", &depthMinMeters_, 0.1f, 50.0f, "%.1f");
    ImGui::SliderFloat("Max Depth (m)", &depthMaxMeters_, 1.0f, 100.0f, "%.1f");
    if (depthMaxMeters_ < depthMinMeters_) depthMaxMeters_ = depthMinMeters_ + 0.1f;

    ImGui::Checkbox("Save raw depth", &depthSaveRaw_);
    if (depthSaveRaw_) {
        const char* rawFmt[] = { "TIFF 32F (.tiff)", "PFM (.pfm)", "EXR (.exr)", "BIN (.bin)" };
        ImGui::Combo("Raw Format", &depthRawFormatIndex_, rawFmt, IM_ARRAYSIZE(rawFmt));
    }
    ImGui::Checkbox("Cache left RGB frames", &depthSaveRgbFrames_);
    ImGui::Checkbox("Save confidence maps", &depthSaveConfidence_);
    ImGui::Checkbox("Save colorized heatmaps (.png)", &depthSaveColorized_);
    ImGui::Checkbox("Create heatmap video (.avi)", &depthSaveVideo_);
    ImGui::Checkbox("Overlay on RGB", &depthOverlayEnabled_);
    ImGui::SliderInt("Overlay Strength (%)", &depthOverlayStrength_, 0, 100);
    ImGui::Checkbox("Auto Contrast (percentiles)", &depthAutoContrast_);
    ImGui::SliderInt("Confidence Threshold", &depthConfidenceThresh_, 0, 100);
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Advanced Visualization", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Edge Emphasis", &depthEdgeBoost_);
        ImGui::SliderFloat("Edge Boost Factor", &depthEdgeFactor_, 0.0f, 2.0f, "%.2f");
        ImGui::Checkbox("CLAHE (local contrast)", &depthClahe_);
        ImGui::Checkbox("Temporal Smoothing (EMA)", &depthTemporal_);
        ImGui::SliderFloat("Temporal Alpha", &depthTemporalAlpha_, 0.05f, 0.8f, "%.2f");
        ImGui::Checkbox("Log Scaling", &depthLogScale_);
        const char* cmap[] = { "Turbo", "Viridis", "Plasma", "Jet" };
        ImGui::Combo("Colormap", &depthColorMapIndex_, cmap, IM_ARRAYSIZE(cmap));
        ImGui::Checkbox("Highlight Motion", &depthHighlightMotion_);
        ImGui::SliderFloat("Motion Gain", &depthMotionGain_, 0.0f, 1.0f, "%.2f");
    }

    ImGui::Separator();

    if (isProcessing_) {
        std::lock_guard<std::mutex> lock(progressMutex_);
        ImGui::ProgressBar(progressValue_, ImVec2(-1, 0), progressMessage_.c_str());
        if (ImGui::Button("Cancel", ImVec2(-1, 30))) {
            cancelExtraction();
        }
    } else {
        if (ImGui::Button("Start Depth Extraction", ImVec2(-1, 40))) {
            startDepthExtraction();
        }
        ImGui::Separator();
        if (ImGui::Button("Open Raw Depth Viewer")) {
            showRawDepthWindow_ = true;
            rawCacheIndex_ = -2; // reset cache on open
            rawCache_.release();
            rawViewerMin_ = depthMinMeters_;
            rawViewerMax_ = depthMaxMeters_;
            confCacheIndex_ = -2;
            confCache8_.release();
            rawViewerRequestFocus_ = true;
        }
        if (navIndex_ >= 0) {
            // Use a unique ID suffix to avoid conflicts with the same-labeled button in the navigator
            if (ImGui::Button("Re-render This Frame (overwrite)##main", ImVec2(-1, 30))) {
                triggerRerenderSelected();
            }
        }
    }
    if (showRawDepthWindow_) {
        renderRawDepthWindow();
    }
}

void GUIApplication::renderStatusBar() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetFrameHeight()));
    
    ImGui::Begin("StatusBar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("ZED SVO2 Extractor v0.1.0 | Ready");
    
    ImGui::End();
}

void GUIApplication::selectSVOFile() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[512] = { 0 };
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = glfwGetWin32Window(window_);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "SVO Files\0*.svo;*.svo2\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn)) {
        svoFilePath_ = szFile;
        // Mirror into buffer for display consistency
        memset(svoPathBuf_, 0, sizeof(svoPathBuf_));
        strncpy(svoPathBuf_, svoFilePath_.c_str(), sizeof(svoPathBuf_) - 1);
    }
#else
    // TODO: Linux/Mac file dialog
    std::cout << "File dialog not implemented for this platform" << std::endl;
#endif
}

void GUIApplication::selectOutputPath() {
#ifdef _WIN32
    BROWSEINFOA bi;
    char szDir[512] = { 0 };
    
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = glfwGetWin32Window(window_);
    bi.pszDisplayName = szDir;
    bi.lpszTitle = "Select Output Directory";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != 0) {
        SHGetPathFromIDListA(pidl, szDir);
        outputPath_ = szDir;
        // Sync buffer with chosen path
        memset(outPathBuf_, 0, sizeof(outPathBuf_));
        strncpy(outPathBuf_, outputPath_.c_str(), sizeof(outPathBuf_) - 1);
        CoTaskMemFree(pidl);
    }
#else
    // TODO: Linux/Mac directory dialog
    std::cout << "Directory dialog not implemented for this platform" << std::endl;
#endif
}

void GUIApplication::startFrameExtraction() {
    if (svoFilePath_.empty()) {
        std::cerr << "No SVO file selected" << std::endl;
        updateProgress(0.0f, "Error: No SVO file selected!");
        return;
    }
    // Sync edited output path buffer back to string
    if (outPathBuf_[0] != '\0') {
        outputPath_ = outPathBuf_;
    }
    
    if (isProcessing_) {
        std::cerr << "Extraction already in progress" << std::endl;
        return;
    }
    
    // Join previous thread if it exists
    if (extractionThread_ && extractionThread_->joinable()) {
        extractionThread_->join();
    }
    
    isProcessing_ = true;
    updateProgress(0.0f, "Starting frame extraction...");
    
    // Build extraction configuration
    zed_extractor::FrameExtractionConfig config;
    config.svoFilePath = svoFilePath_;
    config.baseOutputPath = outputPath_;
    config.fps = frameFps_;
    
    const char* cameras[] = { "left", "right", "both" };
    config.cameraMode = cameras[frameCamera_];
    
    const char* formats[] = { "png", "jpg" };
    config.format = formats[frameFormat_];
    
    // Start extraction in background thread
    extractionThread_ = std::make_unique<std::thread>([this, config]() {
        auto result = engine_->extractFrames(config, 
            [this](float progress, const std::string& message) {
                this->updateProgress(progress, message);
            });
        
        // Store result
        lastResultSuccess_ = result.success;
        if (result.success) {
            lastResultMessage_ = "Frame extraction completed: " + 
                               std::to_string(result.framesProcessed) + " frames extracted";
        } else {
            lastResultMessage_ = "Error: " + result.errorMessage;
        }
    });
}

void GUIApplication::startVideoExtraction() {
    if (svoFilePath_.empty()) {
        std::cerr << "No SVO file selected" << std::endl;
        updateProgress(0.0f, "Error: No SVO file selected!");
        return;
    }
    // Sync edited output path buffer back to string
    if (outPathBuf_[0] != '\0') {
        outputPath_ = outPathBuf_;
    }
    
    if (isProcessing_) {
        std::cerr << "Extraction already in progress" << std::endl;
        return;
    }
    
    // Join previous thread if it exists
    if (extractionThread_ && extractionThread_->joinable()) {
        extractionThread_->join();
    }
    
    isProcessing_ = true;
    updateProgress(0.0f, "Starting video extraction...");
    
    // Build extraction configuration
    zed_extractor::VideoExtractionConfig config;
    config.svoFilePath = svoFilePath_;
    config.baseOutputPath = outputPath_;
    
    const char* cameras[] = { "left", "right", "both_separate", "side_by_side" };
    config.cameraMode = cameras[videoCamera_];
    
    const char* codecs[] = { "h264", "h265", "mjpeg" };
    config.codec = codecs[videoCodec_];
    
    config.outputFps = videoFps_;
    config.quality = videoQuality_;
    
    // Start extraction in background thread
    extractionThread_ = std::make_unique<std::thread>([this, config]() {
        auto result = engine_->extractVideo(config, 
            [this](float progress, const std::string& message) {
                this->updateProgress(progress, message);
            });
        
        // Store result
        lastResultSuccess_ = result.success;
        if (result.success) {
            lastResultMessage_ = "Video extraction completed: " + 
                               std::to_string(result.framesProcessed) + " frames processed";
        } else {
            lastResultMessage_ = "Error: " + result.errorMessage;
        }
    });
}

void GUIApplication::startDepthExtraction() {
    if (svoFilePath_.empty()) {
        std::cerr << "No SVO file selected" << std::endl;
        updateProgress(0.0f, "Error: No SVO file selected!");
        return;
    }
    // Sync edited output path buffer back to string
    if (outPathBuf_[0] != '\0') {
        outputPath_ = outPathBuf_;
    }

    if (isProcessing_) {
        std::cerr << "Extraction already in progress" << std::endl;
        return;
    }

    if (extractionThread_ && extractionThread_->joinable()) {
        extractionThread_->join();
    }

    isProcessing_ = true;
    updateProgress(0.0f, "Starting depth extraction...");

    // Build extraction configuration
    zed_extractor::DepthExtractionConfig config;
    config.svoFilePath = svoFilePath_;
    config.baseOutputPath = outputPath_;
    config.outputFps = depthOutputFps_;
    config.minDepth = depthMinMeters_;
    config.maxDepth = depthMaxMeters_;
    config.saveRawDepth = depthSaveRaw_;
    config.saveColorized = depthSaveColorized_;
    config.saveVideo = depthSaveVideo_;
    config.saveRgbFrames = depthSaveRgbFrames_ && config.overlayOnRgb; // only meaningful if overlay requested
    config.saveConfidenceMaps = depthSaveConfidence_;
    // Raw depth format mapping
    switch (depthRawFormatIndex_) {
        case 0: config.rawDepthFormat = "tiff32f"; break;
        case 1: config.rawDepthFormat = "pfm"; break;
        case 2: config.rawDepthFormat = "exr"; break;
        case 3: config.rawDepthFormat = "bin"; break;
        default: config.rawDepthFormat = "tiff32f"; break;
    }
    const char* modes[] = { "NEURAL", "NEURAL_PLUS", "PERFORMANCE", "QUALITY", "ULTRA" };
    config.depthMode = modes[depthMode_];
    config.overlayOnRgb = depthOverlayEnabled_;
    config.overlayStrength = depthOverlayStrength_;
    config.autoContrast = depthAutoContrast_;
    config.confidenceThreshold = depthConfidenceThresh_;
    config.useEdgeBoost = depthEdgeBoost_;
    config.edgeBoostFactor = depthEdgeFactor_;
    config.useClahe = depthClahe_;
    config.useTemporalSmooth = depthTemporal_;
    config.temporalAlpha = depthTemporalAlpha_;
    config.logScale = depthLogScale_;
    const char* cmaps[] = { "turbo", "viridis", "plasma", "jet" };
    config.colorMap = cmaps[depthColorMapIndex_];
    config.highlightMotion = depthHighlightMotion_;
    config.motionGain = depthMotionGain_;

    extractionThread_ = std::make_unique<std::thread>([this, config]() {
        auto result = engine_->extractDepth(config,
            [this](float progress, const std::string& message) {
                this->updateProgress(progress, message);
            });

        lastResultSuccess_ = result.success;
        if (result.success) {
            lastResultMessage_ = "Depth extraction completed: " +
                                 std::to_string(result.framesProcessed) + " maps saved";
        } else {
            lastResultMessage_ = "Error: " + result.errorMessage;
        }
    });
}

void GUIApplication::cancelExtraction() {
    if (engine_ && isProcessing_) {
        engine_->cancel();
        updateProgress(0.0f, "Cancelling...");
    }
}

void GUIApplication::checkExtractionComplete() {
    if (extractionThread_ && !engine_->isRunning()) {
        if (extractionThread_->joinable()) {
            extractionThread_->join();
        }
        extractionThread_.reset();
        // Do not clear live preview; keep last frame visible
        isProcessing_ = false;
        
        // Show final result
        if (lastResultSuccess_) {
            std::lock_guard<std::mutex> lock(progressMutex_);
            progressMessage_ = lastResultMessage_;
            progressValue_ = 1.0f;
        } else {
            std::lock_guard<std::mutex> lock(progressMutex_);
            if (!progressMessage_.empty()) {
                // Keep last error message instead of reverting to button immediately
                progressValue_ = 0.0f;
            }
        }
    }
}

void GUIApplication::updateProgress(float progress, const std::string& message) {
    progressValue_ = progress;
    std::lock_guard<std::mutex> lock(progressMutex_);
    progressMessage_ = message;
}

void GUIApplication::updateDepthPreview() {
    if (!engine_) return;
    cv::Mat latest;
    int ver = -1;
    if (!engine_->getLatestDepthPreview(latest, ver)) return;

    if (ver == depthPreviewVersion_) return; // no update

    // Ensure 8UC3 BGR
    if (latest.empty()) return;
    if (latest.type() == CV_8UC1) {
        cv::Mat ch[3] = { latest, latest, latest };
        cv::Mat tmp;
        cv::merge(ch, 3, tmp);
        latest = tmp;
    }

    std::lock_guard<std::mutex> lk(depthPreviewMutex_);
    depthPreview_ = latest; // shallow copy ok, we keep Mat alive in member
    depthPreviewWidth_ = depthPreview_.cols;
    depthPreviewHeight_ = depthPreview_.rows;

    // Upload/update OpenGL texture
    if (depthPreviewTexture_ == 0) {
        glGenTextures(1, &depthPreviewTexture_);
        glBindTexture(GL_TEXTURE_2D, depthPreviewTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glBindTexture(GL_TEXTURE_2D, depthPreviewTexture_);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Upload BGR data specifying GL_BGR format
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, depthPreviewWidth_, depthPreviewHeight_, 0, GL_BGR, GL_UNSIGNED_BYTE, depthPreview_.data);
    glBindTexture(GL_TEXTURE_2D, 0);

    depthPreviewVersion_ = ver;

    // Fetch legend info if updated

    // (legend upload continues below)
    zed_extractor::ExtractionEngine::DepthPreviewInfo info;
    int infoVer = -1;
    if (engine_->getLatestDepthPreviewInfo(info, infoVer) && infoVer == ver && infoVer != legendVersionSeen_) {
        legendVersionSeen_ = infoVer;
        legendMinMeters_ = info.minMeters;
        legendMaxMeters_ = info.maxMeters;
        legendAutoContrast_ = info.autoContrast;
        legendLogScale_ = info.logScale;
        legendConfidence_ = info.confidenceThreshold;
        legendColorMap_ = info.colorMap;
        // Pull legend image from engine and upload
        cv::Mat legend;
        int legendVer = -1;
        if (engine_->getLatestDepthLegend(legend, legendVer) && legendVer == ver && !legend.empty()) {
            uploadLegendTexture(legend);
        }
    }
}

static cv::Mat colorizeRedToBlue(const cv::Mat& depth32f, float minD, float maxD) {
    if (depth32f.empty() || depth32f.type() != CV_32FC1) return cv::Mat();
    cv::Mat bgr(depth32f.size(), CV_8UC3);
    float invRange = 1.0f / std::max(1e-6f, maxD - minD);
    for (int y = 0; y < depth32f.rows; ++y) {
        const float* src = depth32f.ptr<float>(y);
        uchar* dst = bgr.ptr<uchar>(y);
        for (int x = 0; x < depth32f.cols; ++x) {
            float d = src[x];
            if (!(d > 0.0f) || !std::isfinite(d)) {
                dst[0] = dst[1] = dst[2] = 0; // invalid -> black
            } else {
                float t = (d - minD) * invRange; // 0 near, 1 far after clamp
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                uchar blue = (uchar)(t * 255.0f);      // far -> blue
                uchar red  = (uchar)((1.0f - t) * 255.0f); // near -> red
                dst[0] = blue;
                dst[1] = 0;
                dst[2] = red;
            }
            dst += 3;
        }
    }
    return bgr;
}

void GUIApplication::renderRawDepthWindow() {
    // Default size and focus when appearing or on request
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 defSize(vp->Size.x * 0.7f, vp->Size.y * 0.7f);
    ImVec2 defPos(vp->Pos.x + vp->Size.x * 0.15f, vp->Pos.y + vp->Size.y * 0.15f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(500, 360), ImVec2(vp->Size.x, vp->Size.y));
    ImGui::SetNextWindowSize(defSize, ImGuiCond_Appearing);
    if (rawViewerRequestFocus_) {
        ImGui::SetNextWindowPos(defPos);
        ImGui::SetNextWindowSize(defSize);
        ImGui::SetNextWindowFocus();
    }
    ImGui::Begin("Raw Depth Viewer", &showRawDepthWindow_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    if (rawViewerRequestFocus_) { ImGui::SetWindowFocus(); rawViewerRequestFocus_ = false; }
    ImGui::Text("View raw 32F depth (near=red, far=blue)");
    // Inline navigation controls for convenience
    if (engine_) {
        int stored = engine_->getStoredPreviewCount();
        if (stored > 0) {
            ImGui::Separator();
            ImGui::Text("Navigate frames");
            ImGui::BeginGroup();
            ImGui::TextDisabled("Total: %d", stored);
            ImGui::SameLine();
            if (ImGui::Button("Live")) { navIndex_ = -1; }
            ImGui::SameLine();
            if (ImGui::Button("Prev")) { if (navIndex_ == -1) navIndex_ = stored - 1; else navIndex_ = (std::max)(0, navIndex_ - navStep_); }
            ImGui::SameLine();
            if (ImGui::Button("Next")) { if (navIndex_ == -1) navIndex_ = 0; else navIndex_ = (std::min)(stored - 1, navIndex_ + navStep_); }
            ImGui::SameLine();
            ImGui::RadioButton("Step1", &navStep_, 1); ImGui::SameLine();
            ImGui::RadioButton("Step5", &navStep_, 5);
            ImGui::SameLine();
            ImGui::Text("Viewing: %s", navIndex_ < 0 ? "Live" : (std::string("#") + std::to_string(navIndex_)).c_str());
            ImGui::EndGroup();
        }
    }
    if (rawViewerMin_ == 0.0f && rawViewerMax_ == 0.0f) {
        rawViewerMin_ = depthMinMeters_;
        rawViewerMax_ = depthMaxMeters_;
    }
    if (rawViewerMin_ >= rawViewerMax_) rawViewerMax_ = rawViewerMin_ + 0.1f;
    ImGui::SliderFloat("Min (m)", &rawViewerMin_, 0.1f, std::max(0.2f, rawViewerMax_-0.1f), "%.2f");
    ImGui::SliderFloat("Max (m)", &rawViewerMax_, rawViewerMin_+0.1f, 200.0f, "%.2f");
    if (rawViewerMax_ - rawViewerMin_ < 0.05f) rawViewerMax_ = rawViewerMin_ + 0.05f;
    ImGui::Checkbox("Auto Apply", &rawViewerAutoApply_);
    static float pendingMin = 0.0f;
    static float pendingMax = 0.0f;
    if (!rawViewerAutoApply_) {
        if (pendingMin == 0.0f && pendingMax == 0.0f) {
            pendingMin = rawViewerMin_;
            pendingMax = rawViewerMax_;
        }
        ImGui::TextDisabled("Adjust sliders then Apply");
        if (ImGui::Button("Apply", ImVec2(80,0))) {
            rawViewerMin_ = pendingMin;
            rawViewerMax_ = pendingMax;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset", ImVec2(80,0))) {
            rawViewerMin_ = depthMinMeters_;
            rawViewerMax_ = depthMaxMeters_;
            pendingMin = rawViewerMin_;
            pendingMax = rawViewerMax_;
        }
    } else {
        // Auto apply: keep pending in sync
        pendingMin = rawViewerMin_;
        pendingMax = rawViewerMax_;
    }
    // Extra debug controls
    ImGui::Separator();
    ImGui::Checkbox("Mask by confidence", &rawViewerUseConfMask_);
    if (rawViewerUseConfMask_) {
        ImGui::SameLine();
        ImGui::SliderInt("Conf. Threshold", &rawViewerConfThresh_, 0, 100);
        if (confCache8_.empty()) {
            ImGui::SameLine(); ImGui::TextColored(ImVec4(1,0.5f,0,1), "(no confidence cached; enable saving & re-run)");
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Log scale", &rawViewerUseLog_);
    ImGui::SameLine();
    ImGui::Checkbox("Auto-contrast", &rawViewerAutoContrast_);
    ImGui::SameLine();
    if (ImGui::Button("Reset View")) { rawZoom_ = 1.0f; rawPan_ = ImVec2(0,0); }
    // RGB overlay option (uses cached left_rgb frames if available)
    ImGui::Separator();
    ImGui::Checkbox("Overlay cached RGB", &rawViewerOverlayRgb_);
    if (rawViewerOverlayRgb_) {
        ImGui::SameLine();
        ImGui::SliderInt("Overlay Strength##raw", &rawViewerOverlayStrength_, 0, 100, "%d%%");
        if (rgbCacheBgr_.empty()) {
            ImGui::SameLine(); ImGui::TextColored(ImVec4(1,0.5f,0,1), "(needs 'Cache left RGB frames' during extraction)");
        }
    }

    // Decide which raw frame to display and use caching to avoid repeated disk loads.
    int targetIndex = (navIndex_ < 0) ? -1 : navIndex_;
    cv::Mat depth32;
    bool haveRaw = false;
    if (targetIndex == -1) {
        // Live latest, but if extraction finished and no live raw, fallback to last stored frame
        if (rawCacheIndex_ != -1) { rawCache_.release(); }
        haveRaw = engine_->getLatestRawDepth(depth32);
        if (!haveRaw && !engine_->isRunning()) {
            int stored = engine_->getStoredPreviewCount();
            if (stored > 0) {
                targetIndex = stored - 1;
            }
        }
        rawCacheIndex_ = -1;
    }
    if (targetIndex >= 0) {
        if (rawCacheIndex_ == targetIndex && !rawCache_.empty()) {
            depth32 = rawCache_.clone();
            haveRaw = true;
        } else {
            zed_extractor::DepthExtractionConfig cfg; // lightweight config for load
            cfg.svoFilePath = svoFilePath_;
            const char* modes[] = { "NEURAL", "NEURAL_PLUS", "PERFORMANCE", "QUALITY", "ULTRA" };
            cfg.depthMode = modes[depthMode_];
            cfg.confidenceThreshold = depthConfidenceThresh_;
            switch (depthRawFormatIndex_) {
                case 0: cfg.rawDepthFormat = "tiff32f"; break;
                case 1: cfg.rawDepthFormat = "pfm"; break;
                case 2: cfg.rawDepthFormat = "exr"; break;
                case 3: cfg.rawDepthFormat = "bin"; break;
                default: cfg.rawDepthFormat = "tiff32f"; break;
            }
            haveRaw = engine_->getDepthFloatForStored(targetIndex, cfg, depth32);
            if (haveRaw) { rawCache_ = depth32.clone(); rawCacheIndex_ = targetIndex; }
        }
        // Load confidence cache if requested
        if (rawViewerUseConfMask_) {
            if (confCacheIndex_ != targetIndex || confCache8_.empty()) {
                cv::Mat conf8; if (engine_->getConfidenceForStored(targetIndex, conf8)) { confCache8_ = conf8; confCacheIndex_ = targetIndex; }
            }
        }
        // Load RGB cache if overlay requested
        if (rawViewerOverlayRgb_) {
            if (rgbCacheIndex_ != targetIndex || rgbCacheBgr_.empty()) {
                cv::Mat rgb; if (engine_->getRgbForStored(targetIndex, rgb)) { rgbCacheBgr_ = rgb; rgbCacheIndex_ = targetIndex; }
            }
        }
    }

    if (haveRaw && !depth32.empty()) {
        // Compute min/max with optional auto-contrast
        float useMin = rawViewerMin_;
        float useMax = rawViewerMax_;
        cv::Mat depthForViz = depth32;
        if (rawViewerUseConfMask_ && !confCache8_.empty()) {
            // Set low-confidence pixels to NaN to be blacked out
            cv::Mat mask = confCache8_ >= rawViewerConfThresh_;
            depthForViz = depth32.clone();
            for (int y = 0; y < depthForViz.rows; ++y) {
                float* d = depthForViz.ptr<float>(y);
                const uchar* m = mask.ptr<uchar>(y);
                for (int x = 0; x < depthForViz.cols; ++x) {
                    if (!m[x]) d[x] = std::numeric_limits<float>::quiet_NaN();
                }
            }
        }
        if (rawViewerAutoContrast_) {
            // Compute simple percentiles of valid finite depths
            std::vector<float> vals; vals.reserve(depthForViz.total());
            for (int y=0;y<depthForViz.rows;++y) {
                const float* d = depthForViz.ptr<float>(y);
                for (int x=0;x<depthForViz.cols;++x) {
                    float v = d[x]; if (std::isfinite(v) && v>0) vals.push_back(v);
                }
            }
            if (vals.size() > 128) {
                std::nth_element(vals.begin(), vals.begin() + vals.size()/50, vals.end());
                float p2 = vals[vals.size()/50];
                std::nth_element(vals.begin(), vals.begin() + vals.size()*49/50, vals.end());
                float p98 = vals[vals.size()*49/50];
                if (p98 > p2) { useMin = p2; useMax = p98; }
            }
        }
        // Black out values outside [useMin,useMax] to match Stereolabs viewer semantics
        {
            for (int y=0; y<depthForViz.rows; ++y) {
                float* d = depthForViz.ptr<float>(y);
                for (int x=0; x<depthForViz.cols; ++x) {
                    float v = d[x];
                    if (!(v>0.0f) || !std::isfinite(v) || v < useMin || v > useMax) {
                        d[x] = std::numeric_limits<float>::quiet_NaN();
                    }
                }
            }
        }
        // Optionally apply log scaling by transforming values into pseudo-depth space
        cv::Mat visInput = depthForViz;
        if (rawViewerUseLog_) {
            cv::Mat logd = depthForViz.clone();
            for (int y=0;y<logd.rows;++y){ float* d=logd.ptr<float>(y); for(int x=0;x<logd.cols;++x){ float v=d[x]; if (std::isfinite(v) && v>0){ d[x] = std::log(std::max(v, 1e-6f)); } }}
            float lmin = std::log(std::max(useMin, 1e-6f));
            float lmax = std::log(std::max(useMax, 1e-6f));
            visInput = logd;
            useMin = lmin; useMax = lmax;
        }
        cv::Mat vis = colorizeRedToBlue(visInput, useMin, useMax);
        // If overlay requested and RGB available, alpha blend
        if (rawViewerOverlayRgb_ && !rgbCacheBgr_.empty()) {
            cv::Mat rgbResized;
            if (rgbCacheBgr_.cols != vis.cols || rgbCacheBgr_.rows != vis.rows) {
                cv::resize(rgbCacheBgr_, rgbResized, vis.size(), 0, 0, cv::INTER_LINEAR);
            } else {
                rgbResized = rgbCacheBgr_;
            }
            double alpha = std::clamp(rawViewerOverlayStrength_ / 100.0, 0.0, 1.0);
            cv::Mat blended; cv::addWeighted(vis, alpha, rgbResized, 1.0 - alpha, 0.0, blended);
            vis = blended;
        }
        if (rawDepthTexture_ == 0) {
            glGenTextures(1, &rawDepthTexture_);
            glBindTexture(GL_TEXTURE_2D, rawDepthTexture_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
            glBindTexture(GL_TEXTURE_2D, rawDepthTexture_);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vis.cols, vis.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, vis.data);
        glBindTexture(GL_TEXTURE_2D, 0);
        rawDepthWidth_ = vis.cols; rawDepthHeight_ = vis.rows;
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float aspect = (float)rawDepthHeight_ / (float)rawDepthWidth_;
        float drawW = avail.x;
        float drawH = drawW * aspect;
        if (drawH > avail.y) { drawH = avail.y; drawW = drawH / aspect; }
        // Canvas for interactions
        ImGui::InvisibleButton("RawDepthCanvas", ImVec2(drawW, drawH), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        ImVec2 canvasPos = ImGui::GetItemRectMin();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mouse = io.MousePos;
        bool hover = mouse.x >= canvasPos.x && mouse.x <= canvasPos.x + drawW && mouse.y >= canvasPos.y && mouse.y <= canvasPos.y + drawH;
        // Zoom with mouse wheel (centered on cursor)
        if (hover && io.MouseWheel != 0.0f) {
            float z0 = rawZoom_;
            float z1 = z0 * (1.0f + io.MouseWheel * 0.2f);
            if (z1 < 0.2f) z1 = 0.2f; if (z1 > 20.0f) z1 = 20.0f;
            if (z1 != z0) {
                ImVec2 local = ImVec2(mouse.x - (canvasPos.x + rawPan_.x), mouse.y - (canvasPos.y + rawPan_.y));
                rawPan_.x -= local.x * (z1 / z0 - 1.0f);
                rawPan_.y -= local.y * (z1 / z0 - 1.0f);
                rawZoom_ = z1;
            }
        }
        // Pan with right mouse drag
        if (hover && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            ImVec2 delta = io.MouseDelta;
            rawPan_.x += delta.x; rawPan_.y += delta.y;
        }
        // Draw image with zoom/pan applied
        ImVec2 destPos = ImVec2(canvasPos.x + rawPan_.x, canvasPos.y + rawPan_.y);
        ImVec2 destMax = ImVec2(destPos.x + drawW * rawZoom_, destPos.y + drawH * rawZoom_);
        dl->AddImage((ImTextureID)(intptr_t)rawDepthTexture_, destPos, destMax);

        // Map mouse to image pixels considering zoom/pan
        float invW = 1.0f / (drawW * rawZoom_);
        float invH = 1.0f / (drawH * rawZoom_);
        if (hover) {
            float u = (mouse.x - destPos.x) * invW;
            float v = (mouse.y - destPos.y) * invH;
            int px = (int)(u * vis.cols);
            int py = (int)(v * vis.rows);
            if (px >=0 && px < vis.cols && py>=0 && py < vis.rows) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    lastPickX_ = px; lastPickY_ = py; float vdepth = depth32.at<float>(py, px); lastPickDepth_ = vdepth; rawSelecting_ = true; rawSelStart_ = mouse; rawSelEnd_ = mouse; }
                if (rawSelecting_ && ImGui::IsMouseDown(ImGuiMouseButton_Left)) { rawSelEnd_ = mouse; }
                if (rawSelecting_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    rawSelecting_ = false;
                    // Compute ROI stats from selection rectangle
                    float u1 = (rawSelStart_.x - destPos.x) * invW; float v1 = (rawSelStart_.y - destPos.y) * invH;
                    float u2 = (rawSelEnd_.x - destPos.x) * invW;   float v2 = (rawSelEnd_.y - destPos.y) * invH;
                    int x1 = (int)(u1 * vis.cols); int y1 = (int)(v1 * vis.rows);
                    int x2 = (int)(u2 * vis.cols); int y2 = (int)(v2 * vis.rows);
                    if (x1>x2) std::swap(x1,x2); if (y1>y2) std::swap(y1,y2);
                    x1 = std::clamp(x1,0,vis.cols-1); x2 = std::clamp(x2,0,vis.cols-1);
                    y1 = std::clamp(y1,0,vis.rows-1); y2 = std::clamp(y2,0,vis.rows-1);
                    double sum=0.0; double mn=1e9; double mx=-1e9; int cnt=0;
                    for(int yy=y1; yy<=y2; ++yy){ const float* d=depth32.ptr<float>(yy); for(int xx=x1; xx<=x2; ++xx){ float val=d[xx]; if (std::isfinite(val) && val>0){ sum+=val; mn=std::min(mn,(double)val); mx=std::max(mx,(double)val); ++cnt; } }}
                    roiX1_=x1; roiY1_=y1; roiX2_=x2; roiY2_=y2; roiCount_=cnt; roiAvg_ = cnt>0 ? (float)(sum/cnt):0.0f; roiMin_ = cnt>0?(float)mn:0.0f; roiMax_ = cnt>0?(float)mx:0.0f;
                }
            }
            // Draw selection rectangle while dragging (in screen space)
            if (rawSelecting_) {
                dl->AddRect(rawSelStart_, rawSelEnd_, IM_COL32(255,255,0,200));
            }
            // Crosshair within canvas
            dl->AddLine(ImVec2(mouse.x, canvasPos.y), ImVec2(mouse.x, canvasPos.y+drawH), IM_COL32(255,255,255,60));
            dl->AddLine(ImVec2(canvasPos.x, mouse.y), ImVec2(canvasPos.x+drawW, mouse.y), IM_COL32(255,255,255,60));
        }
        // Stats panel
        ImGui::Separator();
        ImGui::TextDisabled("Frame: %d", (targetIndex>=0?targetIndex:-1));
        if (lastPickX_ >=0) {
            ImGui::Text("Pick (%d,%d): %.2fm", lastPickX_, lastPickY_, lastPickDepth_);
        }
        if (roiCount_>0) {
            ImGui::Text("ROI (%d,%d)->(%d,%d) pixels=%d avg=%.2fm min=%.2fm max=%.2fm", roiX1_, roiY1_, roiX2_, roiY2_, roiCount_, roiAvg_, roiMin_, roiMax_);
        }
    } else {
        ImGui::TextColored(ImVec4(1,0.7f,0.2f,1), "Raw depth not available yet.");
    }
    ImGui::End();
}

void GUIApplication::renderDepthPreviewPane() {
    std::lock_guard<std::mutex> lk(depthPreviewMutex_);
    if (depthPreviewTexture_ == 0 || depthPreviewWidth_ == 0 || depthPreviewHeight_ == 0) {
        ImGui::TextDisabled("Preview will appear here while extracting...");
        return;
    }
    ImGui::Text("Live Preview (%dx%d)", depthPreviewWidth_, depthPreviewHeight_);
    // Use remaining vertical space dynamically
    float remainingH = ImGui::GetContentRegionAvail().y - 140.0f; // reserve space for legend and controls
    if (remainingH < 120.0f) remainingH = 120.0f; // minimum
    ImGui::BeginChild("DepthPreviewRegion", ImVec2(0, remainingH), true);
    float availW = ImGui::GetContentRegionAvail().x;
    float availH = ImGui::GetContentRegionAvail().y;
    float aspect = (float)depthPreviewHeight_ / (float)depthPreviewWidth_;
    // Fit while preserving aspect
    float drawW = availW;
    float drawH = drawW * aspect;
    if (drawH > availH) {
        drawH = availH;
        drawW = drawH / aspect;
    }
    ImGui::SetCursorPosX((availW - drawW) * 0.5f); // center
    ImGui::Image((void*)(intptr_t)depthPreviewTexture_, ImVec2(drawW, drawH));
    // Legend color bar
    if (legendTexture_ != 0) {
        ImGui::Separator();
        ImGui::Text("Depth Color Scale %s", legendAutoContrast_ ? "(auto)" : "(fixed)");
        ImGui::Image((void*)(intptr_t)legendTexture_, ImVec2(drawW, 20));
        ImGui::Text("Near (hot) %.2fm  |  Far (cool) %.2fm", legendMinMeters_, legendMaxMeters_);
        ImGui::Text("Conf <= %d  Log:%s  Colormap:%s", legendConfidence_, legendLogScale_?"on":"off", legendColorMap_.c_str());
    }
    ImGui::EndChild();
}

void GUIApplication::renderDepthNavigator() {
    // Only show after extraction is finished (not processing) and stored previews exist
    if (isProcessing_) return;
    if (!engine_) return;
    int stored = engine_->getStoredPreviewCount();
    if (stored == 0) return; // nothing to navigate yet
    navCount_ = stored;
    if (navIndex_ >= stored) navIndex_ = stored - 1;
    if (navIndex_ < -1) navIndex_ = -1;

    ImGui::Separator();
    ImGui::Text("Frame Navigation");
    ImGui::BeginGroup();
    ImGui::Text("Stored frames: %d", stored);
    // Step size selector
    ImGui::RadioButton("Step 1", &navStep_, 1); ImGui::SameLine();
    ImGui::RadioButton("Step 5", &navStep_, 5);
    if (ImGui::Button("Live Latest")) { navIndex_ = -1; }
    ImGui::SameLine();
    if (ImGui::Button("<<")) { if (navIndex_ == -1) navIndex_ = stored - 1; else navIndex_ = (std::max)(0, navIndex_ - navStep_); }
    ImGui::SameLine();
    if (ImGui::Button(">>")) { if (navIndex_ == -1) navIndex_ = 0; else navIndex_ = (std::min)(stored - 1, navIndex_ + navStep_); }
    ImGui::SameLine();
    ImGui::Text("Viewing: %s", navIndex_ < 0 ? "Live" : (std::string("#") + std::to_string(navIndex_)).c_str());
    ImGui::EndGroup();

    // If navigating a stored frame, override displayed preview texture with that frame on demand
    if (navIndex_ >= 0) {
        cv::Mat selected;
        if (engine_->getStoredPreviewAt(navIndex_, selected) && !selected.empty()) {
            // Upload to a temp texture (reuse depthPreviewTexture_ for simplicity)
            std::lock_guard<std::mutex> lk2(depthPreviewMutex_);
            depthPreviewWidth_ = selected.cols;
            depthPreviewHeight_ = selected.rows;
            glBindTexture(GL_TEXTURE_2D, depthPreviewTexture_);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, depthPreviewWidth_, depthPreviewHeight_, 0, GL_BGR, GL_UNSIGNED_BYTE, selected.data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // Convenience: allow re-render from within the navigator as well
        // Give this button a different ID than the main-area button to avoid ImGui ID clash
        if (ImGui::Button("Re-render This Frame (overwrite)##nav")) {
            triggerRerenderSelected();
        }
    }

    // Keyboard navigation (Left/Right arrows + Shift for *5)
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard == false && stored > 0) {
        int effectiveStep = navStep_;
        if (io.KeyShift) effectiveStep = 5; // Shift overrides to +5 even if radio at 1
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            if (navIndex_ == -1) navIndex_ = stored - 1; else navIndex_ = (std::max)(0, navIndex_ - effectiveStep);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            if (navIndex_ == -1) navIndex_ = 0; else navIndex_ = (std::min)(stored - 1, navIndex_ + effectiveStep);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R)) { // quick re-render hotkey
            if (navIndex_ >= 0) triggerRerenderSelected();
        }
    }
}

void GUIApplication::triggerRerenderSelected() {
    if (!engine_ || navIndex_ < 0) return;
    // Build config from current UI state
    zed_extractor::DepthExtractionConfig cfg;
    cfg.svoFilePath = svoFilePath_;
    cfg.baseOutputPath = outputPath_;
    cfg.outputFps = depthOutputFps_;
    cfg.minDepth = depthMinMeters_;
    cfg.maxDepth = depthMaxMeters_;
    cfg.saveRawDepth = depthSaveRaw_;
    cfg.saveColorized = depthSaveColorized_;
    cfg.saveVideo = false; // single frame
    const char* modes[] = { "NEURAL", "NEURAL_PLUS", "PERFORMANCE", "QUALITY", "ULTRA" };
    cfg.depthMode = modes[depthMode_];
    cfg.overlayOnRgb = depthOverlayEnabled_;
    cfg.overlayStrength = depthOverlayStrength_;
    cfg.autoContrast = depthAutoContrast_;
    cfg.confidenceThreshold = depthConfidenceThresh_;
    cfg.useEdgeBoost = depthEdgeBoost_;
    cfg.edgeBoostFactor = depthEdgeFactor_;
    cfg.useClahe = depthClahe_;
    cfg.useTemporalSmooth = false; // re-render single frame only
    cfg.temporalAlpha = depthTemporalAlpha_;
    cfg.logScale = depthLogScale_;
    const char* cmaps[] = { "turbo", "viridis", "plasma", "jet" };
    cfg.colorMap = cmaps[depthColorMapIndex_];
    cfg.highlightMotion = false; // single-frame re-render
    cfg.motionGain = depthMotionGain_;
    cfg.storePreviews = true;
    cfg.previewMaxWidth = 960;

    cv::Mat out;
    if (engine_->reprocessDepthFrame(navIndex_, cfg, out, /*overwriteSaved*/true) && !out.empty()) {
        // Update preview texture immediately
        std::lock_guard<std::mutex> lk(depthPreviewMutex_);
        depthPreviewWidth_ = out.cols;
        depthPreviewHeight_ = out.rows;
        glBindTexture(GL_TEXTURE_2D, depthPreviewTexture_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, depthPreviewWidth_, depthPreviewHeight_, 0, GL_BGR, GL_UNSIGNED_BYTE, out.data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

// Fallback GL constants if not provided by loader
#ifndef GL_RGB
#define GL_RGB 0x1907
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif

void GUIApplication::uploadLegendTexture(const cv::Mat& legendBgr) {
    legendWidth_ = legendBgr.cols;
    legendHeight_ = legendBgr.rows;
    if (legendTexture_ == 0) {
        glGenTextures(1, &legendTexture_);
        glBindTexture(GL_TEXTURE_2D, legendTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glBindTexture(GL_TEXTURE_2D, legendTexture_);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, legendWidth_, legendHeight_, 0, GL_BGR, GL_UNSIGNED_BYTE, legendBgr.data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace zed_gui
