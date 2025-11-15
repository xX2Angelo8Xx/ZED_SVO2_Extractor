/**
 * @file extraction_engine.cpp
 * @brief Implementation of unified extraction engine
 * @author ZED SVO2 Extractor Team
 * @date 2025-11-07
 */

#include "extraction_engine.hpp"
#include "error_handler.hpp"
#include "file_utils.hpp"
#include "svo_handler.hpp"
#include "metadata.hpp"
#include "output_manager.hpp"

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <vector>

namespace zed_extractor {

using namespace zed_tools;

// Forward declarations for helpers used by reprocess API
static sl::DEPTH_MODE getDepthMode(const std::string& mode);
static cv::Mat applyDepthHeatmap(const cv::Mat& depthFloat,
                                 float minDepth,
                                 float maxDepth,
                                 bool autoContrast,
                                 const cv::Mat& confidence,
                                 int confidenceThreshold,
                                 bool logScale,
                                 bool useEdgeBoost,
                                 float edgeBoostFactor,
                                 bool useClahe,
                                 const std::string& colorMapName,
                                 double* outA,
                                 double* outB);

// Helper: write PFM (Portable Float Map) grayscale from CV_32FC1
static bool writePFM(const std::string& path, const cv::Mat& depth)
{
    if (depth.empty() || depth.type() != CV_32FC1) return false;
#ifdef _WIN32
    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "wb");
#else
    FILE* f = fopen(path.c_str(), "wb");
#endif
    if (!f) return false;
    // PFM header: Pf (gray), width height, negative scale for little-endian
    fprintf(f, "Pf\n%d %d\n-1.0\n", depth.cols, depth.rows);
    size_t wrote = fwrite(depth.ptr<float>(0), sizeof(float), (size_t)depth.total(), f);
    fclose(f);
    return wrote == (size_t)depth.total();
}

// Helper: read PFM (Portable Float Map) grayscale into CV_32FC1
static cv::Mat readPFM(const std::string& path)
{
#ifdef _WIN32
    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "rb");
#else
    FILE* f = fopen(path.c_str(), "rb");
#endif
    if (!f) return cv::Mat();
    char header[3] = {0};
    if (fread(header, 1, 2, f) != 2) { fclose(f); return cv::Mat(); }
    if (!(header[0] == 'P' && header[1] == 'f')) { fclose(f); return cv::Mat(); }
    int width = 0, height = 0;
    float scale = 0.0f;
    if (fscanf(f, "%d %d\n", &width, &height) != 2) { fclose(f); return cv::Mat(); }
    if (fscanf(f, "%f\n", &scale) != 1) { fclose(f); return cv::Mat(); }
    // Negative scale indicates little-endian floats; absolute value is pixel scale (unused here)
    size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
    cv::Mat depth(height, width, CV_32FC1);
    size_t read = fread(depth.ptr<float>(0), sizeof(float), count, f);
    fclose(f);
    if (read != count) return cv::Mat();
    return depth;
}

/**
 * @brief Convert sl::Mat to cv::Mat
 */
static cv::Mat slMat2cvMat(sl::Mat& input) {
    int cvType = CV_8UC4;
    switch (input.getDataType()) {
        case sl::MAT_TYPE::F32_C1: cvType = CV_32FC1; break;
        case sl::MAT_TYPE::F32_C2: cvType = CV_32FC2; break;
        case sl::MAT_TYPE::F32_C3: cvType = CV_32FC3; break;
        case sl::MAT_TYPE::F32_C4: cvType = CV_32FC4; break;
        case sl::MAT_TYPE::U8_C1: cvType = CV_8UC1; break;
        case sl::MAT_TYPE::U8_C2: cvType = CV_8UC2; break;
        case sl::MAT_TYPE::U8_C3: cvType = CV_8UC3; break;
        case sl::MAT_TYPE::U8_C4: cvType = CV_8UC4; break;
        default: break;
    }
    return cv::Mat(input.getHeight(), input.getWidth(), cvType, input.getPtr<sl::uchar1>());
}

ExtractionEngine::ExtractionEngine()
    : cancelRequested_(false)
    , isRunning_(false)
{
}

bool ExtractionEngine::getLatestDepthPreview(cv::Mat& out, int& version) const {
    std::lock_guard<std::mutex> lock(previewMutex_);
    if (latestPreview_.empty()) return false;
    out = latestPreview_.clone();
    version = previewVersion_.load();
    return true;
}

bool ExtractionEngine::getLatestRawDepth(cv::Mat& out) const {
    std::lock_guard<std::mutex> lock(previewMutex_);
    if (latestRawDepth_.empty()) return false;
    out = latestRawDepth_.clone();
    return true;
}

bool ExtractionEngine::getLatestDepthPreviewInfo(DepthPreviewInfo& out, int& version) const {
    std::lock_guard<std::mutex> lock(previewMutex_);
    if (latestPreview_.empty()) return false; // require at least one preview
    out = latestPreviewInfo_;
    version = previewVersion_.load();
    return true;
}

bool ExtractionEngine::getLatestDepthLegend(cv::Mat& out, int& version) const {
    std::lock_guard<std::mutex> lock(previewMutex_);
    if (latestLegend_.empty()) return false;
    out = latestLegend_.clone();
    version = previewVersion_.load();
    return true;
}

int ExtractionEngine::getStoredPreviewCount() const {
    std::lock_guard<std::mutex> lock(previewMutex_);
    return static_cast<int>(storedPreviews_.size());
}

bool ExtractionEngine::getStoredPreviewAt(int index, cv::Mat& out) const {
    std::lock_guard<std::mutex> lock(previewMutex_);
    if (index < 0 || index >= static_cast<int>(storedPreviews_.size())) return false;
    out = storedPreviews_[index].clone();
    return true;
}

bool ExtractionEngine::setStoredPreviewAt(int index, const cv::Mat& img) {
    std::lock_guard<std::mutex> lock(previewMutex_);
    if (index < 0 || index >= static_cast<int>(storedPreviews_.size())) return false;
    storedPreviews_[index] = img.clone();
    return true;
}

int ExtractionEngine::getStoredFrameIndexAt(int index) const {
    std::lock_guard<std::mutex> lock(previewMutex_);
    if (index < 0 || index >= static_cast<int>(storedFrameIndices_.size())) return -1;
    return storedFrameIndices_[index];
}

bool ExtractionEngine::reprocessDepthFrame(int storedIndex,
                                           const DepthExtractionConfig& cfg,
                                           cv::Mat& outPreview,
                                           bool overwriteSaved) {
    // Attempt to load EXR first if present; otherwise, seek SVO and recompute
    cv::Mat depthFloat;
    cv::Mat confidenceCv;
    int framePos = getStoredFrameIndexAt(storedIndex);
    // Try EXR path if previously saved
    if (!lastExtractionPath_.empty()) {
        std::ostringstream exrName;
        exrName << lastExtractionPath_ << "/depth_maps/depth_" << std::setw(6) << std::setfill('0') << storedIndex << ".exr";
        std::string exrPath = exrName.str();
        cv::Mat exr = cv::imread(exrPath, cv::IMREAD_UNCHANGED);
        if (!exr.empty() && exr.type() == CV_32FC1) {
            depthFloat = exr;
        }
    }
    if (depthFloat.empty()) {
        // Fallback: open SVO and retrieve at framePos
        sl::InitParameters initParams;
        initParams.input.setFromSVOFile(cfg.svoFilePath.c_str());
        initParams.depth_mode = getDepthMode(cfg.depthMode);
        initParams.coordinate_units = sl::UNIT::METER;
        initParams.svo_real_time_mode = false;
        sl::Camera cam;
        if (cam.open(initParams) != sl::ERROR_CODE::SUCCESS) return false;
        if (framePos >= 0) cam.setSVOPosition(framePos);
        sl::RuntimeParameters rp;
        rp.confidence_threshold = cfg.confidenceThreshold;
        rp.texture_confidence_threshold = 100;
        if (cam.grab(rp) != sl::ERROR_CODE::SUCCESS) { cam.close(); return false; }
        sl::Mat depthZed;
        cam.retrieveMeasure(depthZed, sl::MEASURE::DEPTH);
        depthFloat = slMat2cvMat(depthZed).clone();
        sl::Mat confZ;
        cam.retrieveMeasure(confZ, sl::MEASURE::CONFIDENCE);
        confidenceCv = slMat2cvMat(confZ).clone();
        // Optionally get RGB for overlay
        cv::Mat leftBgr;
        if (cfg.overlayOnRgb) {
            sl::Mat leftZ;
            cam.retrieveImage(leftZ, sl::VIEW::LEFT);
            cv::Mat leftRaw = slMat2cvMat(leftZ);
            if (leftRaw.channels() == 4) cv::cvtColor(leftRaw, leftBgr, cv::COLOR_BGRA2BGR);
            else if (leftRaw.channels() == 3) leftBgr = leftRaw.clone();
            else if (leftRaw.channels() == 1) cv::cvtColor(leftRaw, leftBgr, cv::COLOR_GRAY2BGR);
        }
        // Build preview
        double effA = cfg.minDepth, effB = cfg.maxDepth;
        cv::Mat heatmap = applyDepthHeatmap(depthFloat, cfg.minDepth, cfg.maxDepth, cfg.autoContrast,
                                            confidenceCv, cfg.confidenceThreshold, cfg.logScale,
                                            cfg.useEdgeBoost, cfg.edgeBoostFactor, cfg.useClahe,
                                            cfg.colorMap, &effA, &effB);
        cv::Mat out = heatmap;
        if (cfg.overlayOnRgb && !leftBgr.empty()) {
            double alpha = cfg.overlayStrength / 100.0;
            cv::Mat blended; cv::addWeighted(heatmap, alpha, leftBgr, 1.0 - alpha, 0.0, blended);
            out = blended;
        }
        outPreview = out;
        cam.close();
    } else {
        // Have depthFloat from EXR; need confidence map? not available; proceed without confidence mask
        double effA = cfg.minDepth, effB = cfg.maxDepth;
        cv::Mat heatmap = applyDepthHeatmap(depthFloat, cfg.minDepth, cfg.maxDepth, cfg.autoContrast,
                                            cv::Mat(), cfg.confidenceThreshold, cfg.logScale,
                                            cfg.useEdgeBoost, cfg.edgeBoostFactor, cfg.useClahe,
                                            cfg.colorMap, &effA, &effB);
        cv::Mat out = heatmap;
        if (cfg.overlayOnRgb) {
            // Try to load cached RGB from disk first
            cv::Mat leftBgr;
            if (!lastExtractionPath_.empty()) {
                std::ostringstream p;
                p << lastExtractionPath_ << "/left_rgb/left_" << std::setw(6) << std::setfill('0') << storedIndex << ".png";
                cv::Mat tmp = cv::imread(p.str(), cv::IMREAD_COLOR);
                if (!tmp.empty()) leftBgr = tmp;
            }
            if (leftBgr.empty()) {
                // As a last resort, re-seek SVO to fetch RGB (slower)
                sl::InitParameters initParams;
                initParams.input.setFromSVOFile(cfg.svoFilePath.c_str());
                initParams.depth_mode = getDepthMode(cfg.depthMode);
                initParams.svo_real_time_mode = false;
                sl::Camera cam;
                if (cam.open(initParams) == sl::ERROR_CODE::SUCCESS) {
                    if (framePos >= 0) cam.setSVOPosition(framePos);
                    sl::RuntimeParameters rp; rp.confidence_threshold = cfg.confidenceThreshold; rp.texture_confidence_threshold = 100;
                    if (cam.grab(rp) == sl::ERROR_CODE::SUCCESS) {
                        sl::Mat leftZ; cam.retrieveImage(leftZ, sl::VIEW::LEFT);
                        cv::Mat leftRaw = slMat2cvMat(leftZ);
                        if (leftRaw.channels() == 4) cv::cvtColor(leftRaw, leftBgr, cv::COLOR_BGRA2BGR);
                        else if (leftRaw.channels() == 3) leftBgr = leftRaw;
                        else if (leftRaw.channels() == 1) cv::cvtColor(leftRaw, leftBgr, cv::COLOR_GRAY2BGR);
                    }
                    cam.close();
                }
            }
            if (!leftBgr.empty()) {
                double alpha = cfg.overlayStrength / 100.0;
                cv::Mat blended; cv::addWeighted(heatmap, alpha, leftBgr, 1.0 - alpha, 0.0, blended);
                out = blended;
            }
        }
        outPreview = out;
    }

    // Overwrite saved heatmap if requested
    if (overwriteSaved && !lastExtractionPath_.empty() && !outPreview.empty()) {
        std::ostringstream pngName;
        pngName << lastExtractionPath_ << "/depth_heatmaps/heatmap_" << std::setw(6) << std::setfill('0') << storedIndex << ".png";
        cv::imwrite(pngName.str(), outPreview);
    }

    // Update engine latest preview and stored preview entry
    {
        std::lock_guard<std::mutex> lk(previewMutex_);
        latestPreview_ = outPreview.clone();
        if (storedIndex >= 0 && storedIndex < static_cast<int>(storedPreviews_.size())) {
            storedPreviews_[storedIndex] = outPreview.clone();
        }
        ++previewVersion_;
    }
    return !outPreview.empty();
}

bool ExtractionEngine::getDepthFloatForStored(int storedIndex,
                                              const DepthExtractionConfig& cfg,
                                              cv::Mat& outDepthFloat) {
    // First, try to load from disk if we have a recent extraction path
    if (!lastExtractionPath_.empty()) {
        std::string base = lastExtractionPath_ + "/depth_maps/depth_";
        std::ostringstream idx; idx << std::setw(6) << std::setfill('0') << storedIndex;
        base += idx.str();
        // Build candidate list based on preferred format
        std::vector<std::string> exts;
        std::string fmt = cfg.rawDepthFormat.empty() ? std::string("tiff32f") : cfg.rawDepthFormat;
        std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);
        auto push_unique = [&](const std::string& e){ if (std::find(exts.begin(), exts.end(), e) == exts.end()) exts.push_back(e); };
        if (fmt == "tiff32f" || fmt == "tiff") { push_unique(".tiff"); }
        else if (fmt == "pfm") { push_unique(".pfm"); }
        else if (fmt == "exr") { push_unique(".exr"); }
        else if (fmt == "bin") { push_unique(".bin"); }
        // Add other known types as fallbacks
        push_unique(".tiff"); push_unique(".pfm"); push_unique(".exr"); push_unique(".bin");
        for (const auto& ext : exts) {
            std::string path = base + ext;
            if (zed_tools::FileUtils::fileExists(path)) {
                if (ext == ".tiff" || ext == ".exr") {
                    cv::Mat m = cv::imread(path, cv::IMREAD_UNCHANGED);
                    if (!m.empty()) {
                        if (m.type() == CV_32FC1) { outDepthFloat = m; return true; }
                        if (m.channels() == 1) { m.convertTo(outDepthFloat, CV_32FC1); return true; }
                    }
                } else if (ext == ".pfm") {
                    cv::Mat m = readPFM(path);
                    if (!m.empty()) { outDepthFloat = m; return true; }
                } else if (ext == ".bin") {
                    // Cannot infer dimensions reliably; skip BIN load here
                    continue;
                }
            }
        }
    }

    // Fallback: re-seek SVO and retrieve depth
    int framePos = getStoredFrameIndexAt(storedIndex);
    if (framePos < 0) return false;
    try {
        sl::InitParameters initParams;
        initParams.input.setFromSVOFile(cfg.svoFilePath.c_str());
        initParams.depth_mode = getDepthMode(cfg.depthMode);
        initParams.coordinate_units = sl::UNIT::METER;
        initParams.svo_real_time_mode = false;
        sl::Camera cam;
        if (cam.open(initParams) != sl::ERROR_CODE::SUCCESS) return false;
        cam.setSVOPosition(framePos);
        sl::RuntimeParameters rp; rp.confidence_threshold = cfg.confidenceThreshold; rp.texture_confidence_threshold = 100;
        if (cam.grab(rp) != sl::ERROR_CODE::SUCCESS) { cam.close(); return false; }
        sl::Mat depthZed; cam.retrieveMeasure(depthZed, sl::MEASURE::DEPTH);
        cv::Mat df = slMat2cvMat(depthZed);
        if (df.empty() || df.type() != CV_32FC1) { cam.close(); return false; }
        outDepthFloat = df.clone();
        cam.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool ExtractionEngine::getConfidenceForStored(int storedIndex, cv::Mat& outConf8u) const {
    if (lastExtractionPath_.empty()) return false;
    // Try exact match with stored index
    auto buildPath = [&](int idx){
        std::ostringstream p; p << lastExtractionPath_ << "/confidence_maps/conf_" << std::setw(6) << std::setfill('0') << idx << ".png"; return p.str(); };
    std::string path = buildPath(storedIndex);
    cv::Mat m = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (m.empty()) {
        // Fallback: if storedIndex maps to an absolute SVO frame index, try to map by filename prefix pattern if needed
        // Or probe nearby indices in case of off-by-one during extraction windowing (rare)
        for (int d = -2; d <= 2 && m.empty(); ++d) {
            int alt = storedIndex + d; if (alt < 0) continue; m = cv::imread(buildPath(alt), cv::IMREAD_UNCHANGED);
        }
        if (m.empty()) return false;
    }
    if (m.type() != CV_8UC1) {
        cv::Mat gray; m.convertTo(gray, CV_8UC1); m = gray;
    }
    outConf8u = m;
    return true;
}

bool ExtractionEngine::getRgbForStored(int storedIndex, cv::Mat& outBgr) const {
    if (lastExtractionPath_.empty()) return false;
    std::ostringstream p;
    p << lastExtractionPath_ << "/left_rgb/left_" << std::setw(6) << std::setfill('0') << storedIndex << ".png";
    std::string path = p.str();
    cv::Mat m = cv::imread(path, cv::IMREAD_COLOR);
    if (m.empty()) return false;
    outBgr = m;
    return true;
}

ExtractionEngine::~ExtractionEngine() {
    cancel();
}

void ExtractionEngine::cancel() {
    cancelRequested_ = true;
}

bool ExtractionEngine::isRunning() const {
    return isRunning_;
}

bool ExtractionEngine::shouldCancel() const {
    return cancelRequested_;
}

void ExtractionEngine::reportProgress(float progress, const std::string& message, ProgressCallback callback) {
    if (callback) {
        callback(progress, message);
    }
}

ExtractionResult ExtractionEngine::extractFrames(
    const FrameExtractionConfig& config,
    ProgressCallback progressCallback
) {
    if (isRunning_) {
        return ExtractionResult::Failure("Extraction already in progress");
    }
    
    isRunning_ = true;
    cancelRequested_ = false;
    
    try {
        reportProgress(0.0f, "Opening SVO file...", progressCallback);
        
        // Open SVO file using SVOHandler
        SVOHandler svo(config.svoFilePath);
        if (!svo.open()) {
            isRunning_ = false;
            return ExtractionResult::Failure("Failed to open SVO file");
        }
        
        // Get SVO properties
        SVOProperties props = svo.getProperties();
        reportProgress(0.05f, "SVO file opened successfully", progressCallback);
        
        // Get flight folder name from SVO path
        std::string svoPath = config.svoFilePath;
        std::replace(svoPath.begin(), svoPath.end(), '\\', '/');
        
        // Get the parent directory of the SVO file
        size_t lastSlash = svoPath.find_last_of('/');
        std::string parentFolder = (lastSlash != std::string::npos) ? svoPath.substr(0, lastSlash) : "";
        
        std::string flightFolderName = "unknown_flight";
        
        if (!parentFolder.empty()) {
            // Extract just the folder name (last component of path)
            size_t folderNameStart = parentFolder.find_last_of('/');
            std::string folderName = (folderNameStart != std::string::npos) 
                ? parentFolder.substr(folderNameStart + 1) 
                : parentFolder;
            
            // Check if it matches flight folder pattern
            if (FileUtils::isFlightFolder(folderName)) {
                flightFolderName = folderName;
            }
        }
        
        reportProgress(0.08f, "Detected flight: " + flightFolderName, progressCallback);
        
        // Setup output manager
        OutputManager outputMgr(config.baseOutputPath);
        std::string outputPath = outputMgr.getYoloFramesPath(flightFolderName);
        
        if (outputPath.empty()) {
            isRunning_ = false;
            return ExtractionResult::Failure("Failed to create output directory");
        }
        
        reportProgress(0.1f, "Output directory created", progressCallback);
        
        // Calculate frame interval
        int frameInterval = std::max(1, static_cast<int>(std::round(props.fps / config.fps)));
        int svoPosition = 0;
        int frameCount = 0;
        
        sl::Mat image_zed;
        
        // Main extraction loop
        while (svo.grab()) {
            if (shouldCancel()) {
                isRunning_ = false;
                return ExtractionResult::Failure("Extraction cancelled by user");
            }
            
            // Only extract frames at specified interval
            if (svoPosition % frameInterval != 0) {
                svoPosition++;
                continue;
            }
            
            // Extract left camera
            if (config.cameraMode == "left" || config.cameraMode == "both") {
                sl::ERROR_CODE err = svo.retrieveImage(image_zed, sl::VIEW::LEFT);
                if (err == sl::ERROR_CODE::SUCCESS) {
                    int frameNum = outputMgr.getNextGlobalFrameNumber();
                    std::ostringstream filename;
                    filename << "L_frame_" << std::setw(6) << std::setfill('0') << frameNum
                            << "." << config.format;
                    std::string filepath = outputPath + "/" + filename.str();
                    
                    image_zed.write(filepath.c_str());
                    outputMgr.updateGlobalFrameCounter(frameNum);
                    frameCount++;
                }
            }
            
            // Extract right camera
            if (config.cameraMode == "right" || config.cameraMode == "both") {
                sl::ERROR_CODE err = svo.retrieveImage(image_zed, sl::VIEW::RIGHT);
                if (err == sl::ERROR_CODE::SUCCESS) {
                    int frameNum = outputMgr.getNextGlobalFrameNumber();
                    std::ostringstream filename;
                    filename << "R_frame_" << std::setw(6) << std::setfill('0') << frameNum
                            << "." << config.format;
                    std::string filepath = outputPath + "/" + filename.str();
                    
                    image_zed.write(filepath.c_str());
                    outputMgr.updateGlobalFrameCounter(frameNum);
                    frameCount++;
                }
            }
            
            svoPosition++;
            
            // Report progress
            float progress = 0.1f + (0.9f * (svoPosition / static_cast<float>(props.totalFrames)));
            if (frameCount % 10 == 0 || svoPosition % 100 == 0) {
                std::ostringstream msg;
                msg << "Extracting frames: " << frameCount << " extracted";
                reportProgress(progress, msg.str(), progressCallback);
            }
        }
        
        isRunning_ = false;
        reportProgress(1.0f, "Frame extraction completed", progressCallback);
        
        return ExtractionResult::Success(outputPath, frameCount);
        
    } catch (const std::exception& e) {
        isRunning_ = false;
        return ExtractionResult::Failure(std::string("Exception: ") + e.what());
    }
}

ExtractionResult ExtractionEngine::extractVideo(
    const VideoExtractionConfig& config,
    ProgressCallback progressCallback
) {
    if (isRunning_) {
        return ExtractionResult::Failure("Extraction already in progress");
    }
    
    isRunning_ = true;
    cancelRequested_ = false;
    
    try {
        reportProgress(0.0f, "Opening SVO file...", progressCallback);
        
        // Open SVO file using SVOHandler
        SVOHandler svo(config.svoFilePath);
        if (!svo.open()) {
            isRunning_ = false;
            return ExtractionResult::Failure("Failed to open SVO file");
        }
        
        // Get SVO properties
        SVOProperties props = svo.getProperties();
        reportProgress(0.05f, "SVO file opened successfully", progressCallback);
        
        // Get flight folder name from SVO path
        std::string svoPath = config.svoFilePath;
        std::replace(svoPath.begin(), svoPath.end(), '\\', '/');
        
        // Get the parent directory of the SVO file
        size_t lastSlash = svoPath.find_last_of('/');
        std::string parentFolder = (lastSlash != std::string::npos) ? svoPath.substr(0, lastSlash) : "";
        
        std::string flightFolderName = "unknown_flight";
        
        if (!parentFolder.empty()) {
            // Extract just the folder name (last component of path)
            size_t folderNameStart = parentFolder.find_last_of('/');
            std::string folderName = (folderNameStart != std::string::npos) 
                ? parentFolder.substr(folderNameStart + 1) 
                : parentFolder;
            
            // Check if it matches flight folder pattern
            if (FileUtils::isFlightFolder(folderName)) {
                flightFolderName = folderName;
            }
        }
        
        reportProgress(0.08f, "Detected flight: " + flightFolderName, progressCallback);
        
        // Setup output manager
        OutputManager outputMgr(config.baseOutputPath);
        std::string extractionPath = outputMgr.getExtractionPath(flightFolderName, OutputType::VIDEO);
        
        if (extractionPath.empty()) {
            // SVOHandler auto-closes;
            isRunning_ = false;
            return ExtractionResult::Failure("Failed to create extraction directory");
        }
        
        reportProgress(0.1f, "Output directory created", progressCallback);
        
        // Determine output FPS with validation
        float outputFps;
        if (config.outputFps > 0) {
            outputFps = config.outputFps;
            if (outputFps > props.fps) {
                std::ostringstream msg;
                msg << "Requested FPS (" << outputFps << ") exceeds source FPS (" 
                    << props.fps << "), using source FPS";
                reportProgress(0.12f, msg.str(), progressCallback);
                outputFps = props.fps;
            }
        } else {
            outputFps = props.fps;
        }
        
        // Use MJPEG codec which is universally supported
        // MJPEG doesn't require external codecs and always works with OpenCV
        // Files will be larger but 100% compatible
        int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
        std::string extension = ".avi";
        
        // Create video writers
        cv::VideoWriter leftWriter, rightWriter, sideBySideWriter;
        bool writeLeft = (config.cameraMode == "left" || config.cameraMode == "both_separate");
        bool writeRight = (config.cameraMode == "right" || config.cameraMode == "both_separate");
        bool writeSideBySide = (config.cameraMode == "side_by_side");
        
        if (writeLeft) {
            std::string leftPath = extractionPath + "/video_left" + extension;
            leftWriter.open(leftPath, fourcc, outputFps, cv::Size(props.width, props.height), true);
            if (!leftWriter.isOpened()) {
                // SVOHandler auto-closes;
                isRunning_ = false;
                return ExtractionResult::Failure("Failed to create left video writer");
            }
        }
        
        if (writeRight) {
            std::string rightPath = extractionPath + "/video_right" + extension;
            rightWriter.open(rightPath, fourcc, outputFps, cv::Size(props.width, props.height), true);
            if (!rightWriter.isOpened()) {
                // SVOHandler auto-closes;
                isRunning_ = false;
                return ExtractionResult::Failure("Failed to create right video writer");
            }
        }
        
        if (writeSideBySide) {
            std::string sbsPath = extractionPath + "/video_side_by_side" + extension;
            sideBySideWriter.open(sbsPath, fourcc, outputFps, cv::Size(props.width * 2, props.height), true);
            if (!sideBySideWriter.isOpened()) {
                // SVOHandler auto-closes;
                isRunning_ = false;
                return ExtractionResult::Failure("Failed to create side-by-side video writer");
            }
        }
        
        reportProgress(0.15f, "Video writers initialized", progressCallback);
        
        // Main extraction loop
        sl::Mat image_zed_left, image_zed_right;
        cv::Mat image_cv_left, image_cv_right, sideBySide;
        int frameCount = 0;
        
        while (frameCount < props.totalFrames) {
            if (shouldCancel()) {
                leftWriter.release();
                rightWriter.release();
                sideBySideWriter.release();
                // SVOHandler auto-closes;
                isRunning_ = false;
                return ExtractionResult::Failure("Extraction cancelled by user");
            }
            
            if (!svo.grab()) {
                // End of file reached
                break;
            }
            
            // Retrieve left image
            svo.retrieveImage(image_zed_left, sl::VIEW::LEFT);
            cv::Mat image_cv_left_raw = slMat2cvMat(image_zed_left);
            
            // Convert BGRA to BGR if necessary (VideoWriter expects 3-channel BGR)
            if (image_cv_left_raw.channels() == 4) {
                cv::cvtColor(image_cv_left_raw, image_cv_left, cv::COLOR_BGRA2BGR);
            } else {
                image_cv_left = image_cv_left_raw;
            }
            
            if (writeLeft) {
                leftWriter.write(image_cv_left);
            }
            
            if (writeRight || writeSideBySide) {
                svo.retrieveImage(image_zed_right, sl::VIEW::RIGHT);
                cv::Mat image_cv_right_raw = slMat2cvMat(image_zed_right);
                
                // Convert BGRA to BGR if necessary
                if (image_cv_right_raw.channels() == 4) {
                    cv::cvtColor(image_cv_right_raw, image_cv_right, cv::COLOR_BGRA2BGR);
                } else {
                    image_cv_right = image_cv_right_raw;
                }
                
                if (writeRight) {
                    rightWriter.write(image_cv_right);
                }
                
                if (writeSideBySide) {
                    cv::hconcat(image_cv_left, image_cv_right, sideBySide);
                    sideBySideWriter.write(sideBySide);
                }
            }
            
            frameCount++;
            
            // Report progress every 10 frames
            if (frameCount % 10 == 0) {
                float progress = 0.15f + (0.85f * (frameCount / static_cast<float>(props.totalFrames)));
                std::ostringstream msg;
                msg << "Processing: " << frameCount << "/" << props.totalFrames << " frames";
                reportProgress(progress, msg.str(), progressCallback);
            }
        }
        
        // Release resources
        leftWriter.release();
        rightWriter.release();
        sideBySideWriter.release();
        // SVOHandler auto-closes;
        
        isRunning_ = false;
        reportProgress(1.0f, "Video extraction completed", progressCallback);
        
        return ExtractionResult::Success(extractionPath, frameCount);
        
    } catch (const std::exception& e) {
        isRunning_ = false;
        return ExtractionResult::Failure(std::string("Exception: ") + e.what());
    }
}

/**
 * @brief Apply heatmap colorization to depth map
 * Blue = far, Red = close, based on min/max depth range
 */
static int resolveColorMap(const std::string& name) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
#ifdef CV_COLORMAP_TURBO
    if (n == "turbo") return CV_COLORMAP_TURBO;
#endif
    if (n == "viridis") return cv::COLORMAP_VIRIDIS;
    if (n == "plasma") return cv::COLORMAP_PLASMA;
    if (n == "jet") return cv::COLORMAP_JET;
#ifdef CV_COLORMAP_TURBO
    return CV_COLORMAP_TURBO;
#else
    return cv::COLORMAP_JET;
#endif
}

static cv::Mat applyDepthHeatmap(const cv::Mat& depthFloat,
                                 float minDepth,
                                 float maxDepth,
                                 bool autoContrast,
                                 const cv::Mat& confidence,
                                 int confidenceThreshold,
                                 bool logScale,
                                 bool useEdgeBoost,
                                 float edgeBoostFactor,
                                 bool useClahe,
                                 const std::string& colorMapName,
                                 double* outA = nullptr,
                                 double* outB = nullptr) {
    cv::Mat heatmap;
    cv::Mat normalized;
    cv::Mat maskValidBase = (depthFloat >= minDepth) & (depthFloat <= maxDepth) & (depthFloat == depthFloat) & (depthFloat > 0);
    cv::Mat maskValid = maskValidBase.clone();
    if (!confidence.empty()) {
        // ZED confidence: 0 = best, 100 = worst; keep pixels with confidence <= threshold
        cv::Mat confMask = confidence <= confidenceThreshold;
        maskValid = maskValid & confMask;
        // Fallback: if too few valid pixels after confidence filtering, ignore confidence mask
        int validCount = cv::countNonZero(maskValid);
        int minValid = std::max(1000, (depthFloat.rows * depthFloat.cols) / 1000); // ~0.1% or 1000 px
        if (validCount < minValid) {
            maskValid = maskValidBase; // relax to base validity
        }
    }
    // Extract valid depths into vector if autoContrast enabled
    double a = minDepth;
    double b = maxDepth;
    if (autoContrast) {
        std::vector<float> vals;
        vals.reserve(depthFloat.rows * depthFloat.cols / 4);
        for (int y = 0; y < depthFloat.rows; ++y) {
            const float* row = depthFloat.ptr<float>(y);
            const uchar* mrow = maskValid.ptr<uchar>(y);
            for (int x = 0; x < depthFloat.cols; ++x) {
                if (mrow[x]) vals.push_back(row[x]);
            }
        }
        if (vals.size() > 100) { // need enough samples
            auto nth_pct = [&](double pct) -> float {
                size_t idx = static_cast<size_t>(pct * (vals.size() - 1));
                std::nth_element(vals.begin(), vals.begin() + idx, vals.end());
                return vals[idx];
            };
            float p2 = nth_pct(0.02);
            float p98 = nth_pct(0.98);
            if (p98 - p2 > 0.5f) { a = p2; b = p98; }
        }
    }
    // Optionally report chosen bounds
    if (outA) *outA = a;
    if (outB) *outB = b;

    // Scale (linear or log), invert (near-hot), and apply mask
    cv::Mat scaled;
    if (logScale) {
        cv::Mat logd;
        cv::log(depthFloat + 1e-3f, logd);
        double logA = std::log(a + 1e-3);
        double logB = std::log(b + 1e-3);
        scaled = (logd - logA) / (logB - logA);
    } else {
        scaled = (depthFloat - a) / (b - a);
    }
    scaled.setTo(0, depthFloat < a);
    scaled.setTo(1, depthFloat > b);
    scaled = 1.0 - scaled;
    scaled.setTo(0, ~maskValid);

    // Optional edge boost on gradient magnitude
    if (useEdgeBoost) {
        cv::Mat gx, gy, grad;
        cv::Sobel(depthFloat, gx, CV_32F, 1, 0, 3);
        cv::Sobel(depthFloat, gy, CV_32F, 0, 1, 3);
        cv::magnitude(gx, gy, grad);
        cv::Mat gradNorm;
        cv::normalize(grad, gradNorm, 0, 1, cv::NORM_MINMAX);
        cv::Mat boosted = scaled + edgeBoostFactor * gradNorm;
        cv::min(boosted, 1.0, scaled);
        scaled.setTo(0, ~maskValid);
    }

    cv::Mat scaled8;
    scaled.convertTo(scaled8, CV_8UC1, 255.0);

    if (useClahe) {
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8,8));
        clahe->apply(scaled8, scaled8);
    }

    int cmap = resolveColorMap(colorMapName);
    cv::applyColorMap(scaled8, heatmap, cmap);
    heatmap.setTo(cv::Scalar(0,0,0), ~maskValid);
    
    return heatmap;
}

/**
 * @brief Get ZED depth mode from string
 */
static sl::DEPTH_MODE getDepthMode(const std::string& mode) {
    if (mode == "PERFORMANCE") return sl::DEPTH_MODE::PERFORMANCE;
    if (mode == "QUALITY") return sl::DEPTH_MODE::QUALITY;
    if (mode == "ULTRA") return sl::DEPTH_MODE::ULTRA;
    if (mode == "NEURAL") return sl::DEPTH_MODE::NEURAL;
    if (mode == "NEURAL_PLUS") return sl::DEPTH_MODE::NEURAL_PLUS;
    return sl::DEPTH_MODE::NEURAL;  // Default to NEURAL
}

ExtractionResult ExtractionEngine::extractDepth(
    const DepthExtractionConfig& config,
    ProgressCallback progressCallback
) {
    if (isRunning_) {
        return ExtractionResult::Failure("Extraction already in progress");
    }
    
    isRunning_ = true;
    cancelRequested_ = false;
    
    try {
        reportProgress(0.0f, "Initializing depth extraction...", progressCallback);
        
    // Configure camera with depth mode
        sl::InitParameters initParams;
        initParams.input.setFromSVOFile(config.svoFilePath.c_str());
        initParams.depth_mode = getDepthMode(config.depthMode);
        initParams.coordinate_units = sl::UNIT::METER;
        initParams.depth_stabilization = true;
    // Ensure offline playback processes every frame without real-time dropping
    initParams.svo_real_time_mode = false;
        
    sl::Camera camera;
        sl::ERROR_CODE err = camera.open(initParams);
        if (err != sl::ERROR_CODE::SUCCESS) {
            isRunning_ = false;
            return ExtractionResult::Failure("Failed to open SVO file with depth mode: " + 
                                           std::string(sl::toString(err).c_str()));
        }
        
        reportProgress(0.05f, "SVO file opened with depth mode: " + config.depthMode, progressCallback);
        
    // Get SVO properties
    int totalFrames = camera.getSVONumberOfFrames(); // May return 0/1 early; loop will rely on END_OF_SVOFILE_REACHED
        sl::CameraInformation camInfo = camera.getCameraInformation();
        int width = camInfo.camera_configuration.resolution.width;
        int height = camInfo.camera_configuration.resolution.height;
        float sourceFps = camera.getCameraInformation().camera_configuration.fps;
        
        // Get flight folder name from SVO path
        std::string svoPath = config.svoFilePath;
        std::replace(svoPath.begin(), svoPath.end(), '\\', '/');
        
        size_t lastSlash = svoPath.find_last_of('/');
        std::string parentFolder = (lastSlash != std::string::npos) ? svoPath.substr(0, lastSlash) : "";
        
        std::string flightFolderName = "unknown_flight";
        if (!parentFolder.empty()) {
            size_t folderNameStart = parentFolder.find_last_of('/');
            std::string folderName = (folderNameStart != std::string::npos) 
                ? parentFolder.substr(folderNameStart + 1) 
                : parentFolder;
            
            if (FileUtils::isFlightFolder(folderName)) {
                flightFolderName = folderName;
            }
        }
        
        reportProgress(0.08f, "Detected flight: " + flightFolderName, progressCallback);
        
    // Setup output manager
        OutputManager outputMgr(config.baseOutputPath);
    std::string extractionPath = outputMgr.getExtractionPath(flightFolderName, OutputType::DEPTH);
    lastExtractionPath_ = extractionPath;
        
        if (extractionPath.empty()) {
            camera.close();
            isRunning_ = false;
            return ExtractionResult::Failure("Failed to create extraction directory");
        }
        
    // Create subdirectories
    std::string depthDir = extractionPath + "/depth_maps";
    std::string heatmapDir = extractionPath + "/depth_heatmaps";
    std::string rgbDir = extractionPath + "/left_rgb";
    std::string confDir = extractionPath + "/confidence_maps";
        FileUtils::createDirectory(depthDir);
        FileUtils::createDirectory(heatmapDir);
    if (config.saveRgbFrames) FileUtils::createDirectory(rgbDir);
    if (config.saveConfidenceMaps) FileUtils::createDirectory(confDir);
        
        reportProgress(0.1f, "Output directories created", progressCallback);
        
        // Calculate frame interval
        int frameInterval = std::max(1, static_cast<int>(std::round(sourceFps / config.outputFps)));
        
        // Prepare video writer if requested
        cv::VideoWriter videoWriter;
        if (config.saveVideo) {
            std::string videoPath = extractionPath + "/depth_heatmap.avi";
            int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
            videoWriter.open(videoPath, fourcc, config.outputFps, cv::Size(width, height), true);
            
            if (!videoWriter.isOpened()) {
                camera.close();
                isRunning_ = false;
                return ExtractionResult::Failure("Failed to create depth video writer");
            }
        }
        
        reportProgress(0.15f, "Starting depth extraction...", progressCallback);

        // Reset stored previews
        {
            std::lock_guard<std::mutex> lk(previewMutex_);
            storedPreviews_.clear();
            storedFrameIndices_.clear();
        }
        
    // Main extraction loop
    sl::Mat depthZed;
    sl::Mat leftImageZed; // for overlay
    cv::Mat emaDepth;     // temporal smoothing
        int frameCount = 0;
        int extractedCount = 0;
        
    sl::RuntimeParameters runtime_params;
    runtime_params.confidence_threshold = config.confidenceThreshold; // drives internal filtering
    runtime_params.texture_confidence_threshold = 100;
        
    cv::Mat prevDepthForMotion; // store previous depth for motion highlighting
    for (;;) {
            if (shouldCancel()) {
                videoWriter.release();
                camera.close();
                isRunning_ = false;
                return ExtractionResult::Failure("Extraction cancelled by user");
            }

            // Debug: emit a warning if we loop too few times (helps diagnose immediate termination)
            // We only log the first 3 grab attempts to avoid flooding.
            if (frameCount < 3) {
                LOG_INFO("Depth loop iteration (pre-grab) frameCount=" + std::to_string(frameCount));
            }
            
            // Grab frame
            sl::ERROR_CODE grabEc = camera.grab(runtime_params);
            if (grabEc == sl::ERROR_CODE::END_OF_SVOFILE_REACHED) {
                if (frameCount == 0 && extractedCount == 0) {
                    // Immediate end-of-file on first grab: likely wrong path (opened live camera instead of SVO)
                    LOG_ERROR("END_OF_SVOFILE_REACHED on first grab. Check that selected path is a valid .svo/.svo2 file: " + config.svoFilePath);
                }
                break; // normal termination
            }
            if (grabEc != sl::ERROR_CODE::SUCCESS) {
                // Transient read error: try next iteration
                if (frameCount < 3) {
                    LOG_WARNING("Transient grab error: " + std::string(sl::toString(grabEc).c_str()));
                }
                continue;
            }
            
            // Only extract at specified interval
            if (frameInterval > 1 && (frameCount % frameInterval) != 0) {
                frameCount++;
                continue;
            }
            
            // Retrieve depth and confidence maps
            camera.retrieveMeasure(depthZed, sl::MEASURE::DEPTH);
            cv::Mat depthFloat = slMat2cvMat(depthZed);
            sl::Mat confidenceZed;
            camera.retrieveMeasure(confidenceZed, sl::MEASURE::CONFIDENCE);
            cv::Mat confidenceCv = slMat2cvMat(confidenceZed);

            if (depthFloat.empty()) {
                frameCount++;
                continue;
            }

            cv::Mat leftBgr;
            if (config.overlayOnRgb) {
                camera.retrieveImage(leftImageZed, sl::VIEW::LEFT);
                cv::Mat leftRaw = slMat2cvMat(leftImageZed);
                if (!leftRaw.empty()) {
                    if (leftRaw.channels() == 4) {
                        cv::cvtColor(leftRaw, leftBgr, cv::COLOR_BGRA2BGR);
                    } else if (leftRaw.channels() == 3) {
                        leftBgr = leftRaw;
                    } else if (leftRaw.channels() == 1) {
                        cv::cvtColor(leftRaw, leftBgr, cv::COLOR_GRAY2BGR);
                    }
                }
            }
            
            // Save raw depth if requested; support multiple formats
            if (config.saveRawDepth) {
                static bool s_exrWriteAllowed = true;
                static bool s_exrWarnedOnce = false;
                std::string fmt = config.rawDepthFormat;
                std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);
                if (fmt == "auto" || fmt == "exr") {
                    if (s_exrWriteAllowed) {
                        std::ostringstream filenameRaw;
                        filenameRaw << "depth_" << std::setw(6) << std::setfill('0') << extractedCount << ".exr";
                        std::string rawPath = depthDir + "/" + filenameRaw.str();
                        try {
#if CV_VERSION_MAJOR >= 4
                            if (cv::haveImageWriter(".exr")) {
                                if (!cv::imwrite(rawPath, depthFloat)) {
                                    LOG_WARNING("OpenCV failed to write EXR: " + rawPath);
                                    s_exrWriteAllowed = false;
                                }
                            } else {
                                if (!s_exrWarnedOnce) {
                                    LOG_WARNING("OpenEXR codec disabled; skipping EXR saves for this run.");
                                    s_exrWarnedOnce = true;
                                }
                                s_exrWriteAllowed = false;
                            }
#else
                            cv::imwrite(rawPath, depthFloat);
#endif
                        } catch (const std::exception& e) {
                            if (!s_exrWarnedOnce) {
                                LOG_WARNING(std::string("EXR write error; disabling further EXR saves: ") + e.what());
                                s_exrWarnedOnce = true;
                            }
                            s_exrWriteAllowed = false;
                        }
                    }
                } else if (fmt == "tiff32f" || fmt == "tiff") {
                    std::ostringstream filenameRaw;
                    filenameRaw << "depth_" << std::setw(6) << std::setfill('0') << extractedCount << ".tiff";
                    std::string rawPath = depthDir + "/" + filenameRaw.str();
                    try {
                        if (!cv::imwrite(rawPath, depthFloat)) {
                            LOG_WARNING("Failed to write TIFF 32F: " + rawPath);
                        }
                    } catch (const std::exception& e) {
                        LOG_WARNING(std::string("TIFF write error: ") + e.what());
                    }
                } else if (fmt == "pfm") {
                    std::ostringstream filenameRaw;
                    filenameRaw << "depth_" << std::setw(6) << std::setfill('0') << extractedCount << ".pfm";
                    std::string rawPath = depthDir + "/" + filenameRaw.str();
                    if (!writePFM(rawPath, depthFloat)) {
                        LOG_WARNING("Failed to write PFM: " + rawPath);
                    }
                } else if (fmt == "bin") {
                    std::ostringstream filenameRaw;
                    filenameRaw << "depth_" << std::setw(6) << std::setfill('0') << extractedCount << ".bin";
                    std::string rawPath = depthDir + "/" + filenameRaw.str();
#ifdef _WIN32
                    FILE* f = nullptr; fopen_s(&f, rawPath.c_str(), "wb");
#else
                    FILE* f = fopen(rawPath.c_str(), "wb");
#endif
                    if (f) {
                        fwrite(depthFloat.ptr<float>(0), sizeof(float), (size_t)depthFloat.total(), f);
                        fclose(f);
                    } else {
                        LOG_WARNING("Failed to open for BIN write: " + rawPath);
                    }
                }
            }

            // Optionally save left RGB for fast re-render overlay
            if (config.saveRgbFrames && !leftBgr.empty()) {
                std::ostringstream lf;
                lf << rgbDir << "/left_" << std::setw(6) << std::setfill('0') << extractedCount << ".png";
                try { cv::imwrite(lf.str(), leftBgr); } catch (...) {}
            }
            // Optionally save confidence map for debugging (convert to 8-bit if needed)
            if (config.saveConfidenceMaps && !confidenceCv.empty()) {
                cv::Mat conf8;
                if (confidenceCv.type() == CV_8UC1) conf8 = confidenceCv;
                else {
                    double minv=0.0, maxv=0.0; cv::minMaxLoc(confidenceCv, &minv, &maxv);
                    double scale = (maxv > 0.0) ? (255.0 / maxv) : 1.0;
                    confidenceCv.convertTo(conf8, CV_8UC1, scale);
                }
                std::ostringstream cf;
                cf << confDir << "/conf_" << std::setw(6) << std::setfill('0') << extractedCount << ".png";
                try { cv::imwrite(cf.str(), conf8); } catch (...) {}
            }
            
            // Generate colorized heatmap (and optionally save)
            if (config.saveColorized) {
                // Temporal smoothing if enabled
                cv::Mat depthForViz = depthFloat;
                if (config.useTemporalSmooth) {
                    if (emaDepth.empty()) {
                        emaDepth = depthFloat.clone();
                    } else {
                        emaDepth = config.temporalAlpha * depthFloat + (1.0f - config.temporalAlpha) * emaDepth;
                    }
                    depthForViz = emaDepth;
                }
                double effA= config.minDepth, effB = config.maxDepth;
                cv::Mat heatmap = applyDepthHeatmap(
                    depthForViz,
                    config.minDepth,
                    config.maxDepth,
                    config.autoContrast,
                    confidenceCv,
                    config.confidenceThreshold,
                    config.logScale,
                    config.useEdgeBoost,
                    config.edgeBoostFactor,
                    config.useClahe,
                    config.colorMap,
                    &effA,
                    &effB
                );
                // Motion highlight (difference from previous depth)
                if (config.highlightMotion && !prevDepthForMotion.empty() && prevDepthForMotion.size() == depthForViz.size()) {
                    cv::Mat diff;
                    cv::absdiff(depthForViz, prevDepthForMotion, diff);
                    // Normalize diff within valid mask region
                    double maxDiff = 0.0; cv::minMaxLoc(diff, nullptr, &maxDiff);
                    if (maxDiff > 1e-3) {
                        cv::Mat diffNorm = diff / maxDiff; // 0..1
                        // Threshold and dilate to create salient region mask
                        cv::Mat motionMask;
                        cv::threshold(diffNorm, motionMask, 0.15, 1.0, cv::THRESH_BINARY);
                        motionMask.convertTo(motionMask, CV_8UC1, 255.0);
                        cv::dilate(motionMask, motionMask, cv::Mat(), cv::Point(-1,-1), 1);
                        // Apply highlight by blending toward white where motion occurs
                        std::vector<cv::Mat> ch; cv::split(heatmap, ch);
                        for (int c = 0; c < 3; ++c) {
                            ch[c].setTo(ch[c] * (1.0f - config.motionGain) + 255.0f * config.motionGain, motionMask);
                        }
                        cv::merge(ch, heatmap);
                    }
                }
                cv::Mat outputImage = heatmap;
                if (config.overlayOnRgb && !leftBgr.empty()) {
                    double alpha = config.overlayStrength / 100.0;
                    // alpha = fraction of heatmap; (1-alpha) = RGB
                    cv::Mat blended;
                    cv::addWeighted(heatmap, alpha, leftBgr, 1.0 - alpha, 0.0, blended);
                    outputImage = blended;
                }
                prevDepthForMotion = depthForViz.clone();
                // Update live preview (blended or plain heatmap) and legend
                {
                    std::lock_guard<std::mutex> lk(previewMutex_);
                    latestRawDepth_ = depthFloat.clone();
                    latestPreview_ = outputImage.clone();
                    latestPreviewInfo_.minMeters = effA;
                    latestPreviewInfo_.maxMeters = effB;
                    latestPreviewInfo_.autoContrast = config.autoContrast;
                    latestPreviewInfo_.logScale = config.logScale;
                    latestPreviewInfo_.confidenceThreshold = config.confidenceThreshold;
                    latestPreviewInfo_.overlayOnRgb = config.overlayOnRgb;
                    latestPreviewInfo_.overlayStrength = config.overlayStrength;
                    latestPreviewInfo_.colorMap = config.colorMap;
                    // Build legend colorbar (BGR)
                    cv::Mat grad(1, 256, CV_8UC1);
                    for (int x = 0; x < 256; ++x) grad.at<uchar>(0, x) = static_cast<uchar>(x);
                    int cmap = resolveColorMap(config.colorMap);
                    cv::Mat bar;
                    cv::applyColorMap(grad, bar, cmap);
                    cv::resize(bar, latestLegend_, cv::Size(256, 16), 0, 0, cv::INTER_NEAREST);
                    ++previewVersion_;

                    // Store preview if enabled
                    if (config.storePreviews) {
                        cv::Mat toStore = outputImage;
                        if (config.previewMaxWidth > 0 && outputImage.cols > config.previewMaxWidth) {
                            double scale = static_cast<double>(config.previewMaxWidth) / static_cast<double>(outputImage.cols);
                            int newH = static_cast<int>(std::round(outputImage.rows * scale));
                            cv::resize(outputImage, toStore, cv::Size(config.previewMaxWidth, newH));
                        } else {
                            toStore = outputImage.clone();
                        }
                        storedPreviews_.push_back(std::move(toStore));
                        storedFrameIndices_.push_back(frameCount);
                    }
                }
                std::ostringstream filenameHeatmap;
                filenameHeatmap << "heatmap_" << std::setw(6) << std::setfill('0') << extractedCount << ".png";
                std::string heatmapPath = heatmapDir + "/" + filenameHeatmap.str();
                cv::imwrite(heatmapPath, outputImage);
                if (config.saveVideo && videoWriter.isOpened()) {
                    videoWriter.write(outputImage);
                }
            }
            else {
                // Still compute for preview even if not saving colorized
                cv::Mat depthForViz = depthFloat;
                if (config.useTemporalSmooth) {
                    if (emaDepth.empty()) {
                        emaDepth = depthFloat.clone();
                    } else {
                        emaDepth = config.temporalAlpha * depthFloat + (1.0f - config.temporalAlpha) * emaDepth;
                    }
                    depthForViz = emaDepth;
                }
                double effA= config.minDepth, effB = config.maxDepth;
                cv::Mat heatmap = applyDepthHeatmap(
                    depthForViz,
                    config.minDepth,
                    config.maxDepth,
                    config.autoContrast,
                    confidenceCv,
                    config.confidenceThreshold,
                    config.logScale,
                    config.useEdgeBoost,
                    config.edgeBoostFactor,
                    config.useClahe,
                    config.colorMap,
                    &effA,
                    &effB
                );
                if (config.highlightMotion && !prevDepthForMotion.empty() && prevDepthForMotion.size() == depthForViz.size()) {
                    cv::Mat diff;
                    cv::absdiff(depthForViz, prevDepthForMotion, diff);
                    double maxDiff = 0.0; cv::minMaxLoc(diff, nullptr, &maxDiff);
                    if (maxDiff > 1e-3) {
                        cv::Mat diffNorm = diff / maxDiff;
                        cv::Mat motionMask;
                        cv::threshold(diffNorm, motionMask, 0.15, 1.0, cv::THRESH_BINARY);
                        motionMask.convertTo(motionMask, CV_8UC1, 255.0);
                        cv::dilate(motionMask, motionMask, cv::Mat(), cv::Point(-1,-1), 1);
                        std::vector<cv::Mat> ch; cv::split(heatmap, ch);
                        for (int c = 0; c < 3; ++c) {
                            ch[c].setTo(ch[c] * (1.0f - config.motionGain) + 255.0f * config.motionGain, motionMask);
                        }
                        cv::merge(ch, heatmap);
                    }
                }
                cv::Mat outputImage = heatmap;
                if (config.overlayOnRgb && !leftBgr.empty()) {
                    double alpha = config.overlayStrength / 100.0;
                    cv::Mat blended;
                    cv::addWeighted(heatmap, alpha, leftBgr, 1.0 - alpha, 0.0, blended);
                    outputImage = blended;
                }
                {
                    std::lock_guard<std::mutex> lk(previewMutex_);
                    latestRawDepth_ = depthFloat.clone();
                    latestPreview_ = outputImage.clone();
                    prevDepthForMotion = depthForViz.clone();
                    latestPreviewInfo_.minMeters = effA;
                    latestPreviewInfo_.maxMeters = effB;
                    latestPreviewInfo_.autoContrast = config.autoContrast;
                    latestPreviewInfo_.logScale = config.logScale;
                    latestPreviewInfo_.confidenceThreshold = config.confidenceThreshold;
                    latestPreviewInfo_.overlayOnRgb = config.overlayOnRgb;
                    latestPreviewInfo_.overlayStrength = config.overlayStrength;
                    latestPreviewInfo_.colorMap = config.colorMap;
                    // Build legend colorbar (BGR)
                    cv::Mat grad(1, 256, CV_8UC1);
                    for (int x = 0; x < 256; ++x) grad.at<uchar>(0, x) = static_cast<uchar>(x);
                    int cmap = resolveColorMap(config.colorMap);
                    cv::Mat bar;
                    cv::applyColorMap(grad, bar, cmap);
                    cv::resize(bar, latestLegend_, cv::Size(256, 16), 0, 0, cv::INTER_NEAREST);
                    ++previewVersion_;

                    // Store preview if enabled
                    if (config.storePreviews) {
                        cv::Mat toStore = outputImage;
                        if (config.previewMaxWidth > 0 && outputImage.cols > config.previewMaxWidth) {
                            double scale = static_cast<double>(config.previewMaxWidth) / static_cast<double>(outputImage.cols);
                            int newH = static_cast<int>(std::round(outputImage.rows * scale));
                            cv::resize(outputImage, toStore, cv::Size(config.previewMaxWidth, newH));
                        } else {
                            toStore = outputImage.clone();
                        }
                        storedPreviews_.push_back(std::move(toStore));
                        storedFrameIndices_.push_back(frameCount);
                    }
                }
            }
            
            extractedCount++;
            frameCount++;
            
            // Report progress every N frames
            if (extractedCount % 5 == 0) {
                float denom = (totalFrames > 1 ? static_cast<float>(totalFrames) : static_cast<float>(frameCount+1));
                float progress = 0.15f + (0.85f * (frameCount / denom));
                std::ostringstream msg;
                msg << "Extracted: " << extractedCount << " depth maps (frame " << frameCount << ")";
                reportProgress(progress, msg.str(), progressCallback);
            }
        }
        
        // Release resources
        videoWriter.release();
        camera.close();
        
        // Export metadata
        DepthMetadata metadata;
        metadata.extractionDateTime = getCurrentDateTime();
        if (FileUtils::isFlightFolder(flightFolderName)) {
            FlightInfo fi;
            fi.folderName = flightFolderName;
            fi.svoFilePath = config.svoFilePath;
            fi.parseFromFolder(parentFolder); // parentFolder holds the flight folder path
            metadata.flightInfo = fi;
        }
        metadata.width = width;
        metadata.height = height;
        metadata.fps = config.outputFps;
        metadata.totalFrames = extractedCount;
        metadata.neuralMode = config.depthMode;
        metadata.cameraView = "left"; // depth currently from left view by default
        metadata.minDepthMeters = config.minDepth;
        metadata.maxDepthMeters = config.maxDepth;
    metadata.overlayTransparency = config.overlayStrength;
    metadata.showOverlay = config.overlayOnRgb;
        metadata.minObjectPixels = 0;
        metadata.statistics.minDetectedDistance = 0.0f;
        metadata.statistics.maxDetectedDistance = 0.0f;
        metadata.statistics.avgDetectedDistance = 0.0f;
        metadata.statistics.totalObjectsDetected = 0;
        metadata.statistics.framesWithDetections = 0;
        metadata.outputVideo = (config.saveVideo ? extractionPath + "/depth_heatmap.avi" : "");
        
        std::string metadataPath = extractionPath + "/depth_metadata.json";
        metadata.saveToJSON(metadataPath);
        
        if (extractedCount == 0) {
            // Provide actionable failure instead of silent completion
            isRunning_ = false;
            std::string msg = "No depth frames extracted. Possible causes: invalid SVO file path, file contains 0 frames, or SDK opened live camera instead of SVO. Path: " + config.svoFilePath;
            reportProgress(0.0f, msg, progressCallback);
            return ExtractionResult::Failure(msg);
        }

        isRunning_ = false;
        reportProgress(1.0f, "Depth extraction completed", progressCallback);
        return ExtractionResult::Success(extractionPath, extractedCount);
        
    } catch (const std::exception& e) {
        isRunning_ = false;
        return ExtractionResult::Failure(std::string("Exception: ") + e.what());
    }
}

} // namespace zed_extractor
