/**
 * @file gui_application.cpp
 * @brief Implementation of GUI Application
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "gui_application.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <commdlg.h>  // For file dialogs
#include <shlobj.h>   // For folder dialogs
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
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
    , isProcessing_(false)
    , progressValue_(0.0f)
    , progressMessage_("")
    , engine_(nullptr)
    , extractionThread_(nullptr)
    , lastResultMessage_("")
    , lastResultSuccess_(false)
{
    engine_ = std::make_unique<zed_extractor::ExtractionEngine>();
}

GUIApplication::~GUIApplication() {
    if (extractionThread_ && extractionThread_->joinable()) {
        engine_->cancel();
        extractionThread_->join();
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
    ImGui::InputText("##svopath", &svoFilePath_[0], 512, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##svo")) {
        selectSVOFile();
    }

    ImGui::Text("Output Path:");
    ImGui::SameLine();
    ImGui::InputText("##outpath", &outputPath_[0], 512);
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
    ImGui::Text("Extract depth data (Coming Soon)");
    ImGui::Separator();
    
    ImGui::TextWrapped("Depth extraction will be available in a future version. "
                      "This feature will extract depth maps and point clouds from the SVO file.");
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
    // TODO: Implement depth extraction
    std::cout << "Depth extraction not yet implemented" << std::endl;
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
        isProcessing_ = false;
        
        // Show final result
        if (lastResultSuccess_) {
            std::lock_guard<std::mutex> lock(progressMutex_);
            progressMessage_ = lastResultMessage_;
            progressValue_ = 1.0f;
        }
    }
}

void GUIApplication::updateProgress(float progress, const std::string& message) {
    progressValue_ = progress;
    std::lock_guard<std::mutex> lock(progressMutex_);
    progressMessage_ = message;
}

} // namespace zed_gui
