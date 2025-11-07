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

namespace zed_extractor {

using namespace zed_tools;

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
                    filename << "frame_" << std::setw(8) << std::setfill('0') << frameNum 
                            << "_left." << config.format;
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
                    filename << "frame_" << std::setw(8) << std::setfill('0') << frameNum 
                            << "_right." << config.format;
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

} // namespace zed_extractor
