/**
 * @file video_extractor_cli.cpp
 * @brief Command-line video extractor for ZED SVO2 files
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 * 
 * Extracts video from SVO2 files with support for:
 * - Left/Right/Both cameras
 * - Side-by-side stereo output
 * - H.264/H.265 encoding
 * - Custom FPS and resolution
 * 
 * Usage:
 *   video_extractor_cli <svo_file> [options]
 * 
 * Options:
 *   --output-dir <path>     Output directory (default: ./extracted_videos)
 *   --camera <mode>         Camera: left, right, both_separate, side_by_side (default: left)
 *   --codec <codec>         Codec: h264, h265 (default: h264)
 *   --fps <rate>            Output FPS (default: source FPS)
 *   --quality <0-100>       Video quality (default: 90)
 *   --help                  Show this help message
 */

#include <iostream>
#include <string>
#include <cmath>
#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>

// Our common utilities
#include "../../common/error_handler.hpp"
#include "../../common/file_utils.hpp"
#include "../../common/svo_handler.hpp"
#include "../../common/metadata.hpp"
#include "../../common/output_manager.hpp"

using namespace zed_tools;

/**
 * @brief Application configuration
 */
struct Config {
    std::string svoFilePath;
    std::string baseOutputPath = "E:/Turbulence Solutions/AeroLock/ZED_Recordings_Output";
    std::string cameraMode = "left";  // left, right, both_separate, side_by_side
    std::string codec = "h264";       // h264, h265
    float outputFps = -1.0f;          // -1 = use source FPS
    int quality = 90;                 // 0-100
    bool showHelp = false;
};

/**
 * @brief Parse command-line arguments
 */
bool parseArguments(int argc, char* argv[], Config& config) {
    // Check for help flag first
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            config.showHelp = true;
            return true;
        }
    }
    
    // Require SVO file path
    if (argc < 2) {
        std::cerr << "Error: SVO file path required" << std::endl;
        config.showHelp = true;
        return false;
    }
    
    config.svoFilePath = argv[1];
    
    // Parse optional arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--base-output" && i + 1 < argc) {
            config.baseOutputPath = argv[++i];
        }
        else if (arg == "--camera" && i + 1 < argc) {
            config.cameraMode = argv[++i];
        }
        else if (arg == "--codec" && i + 1 < argc) {
            config.codec = argv[++i];
        }
        else if (arg == "--fps" && i + 1 < argc) {
            config.outputFps = std::stof(argv[++i]);
        }
        else if (arg == "--quality" && i + 1 < argc) {
            config.quality = std::stoi(argv[++i]);
        }
        else {
            std::cerr << "Warning: Unknown argument: " << arg << std::endl;
        }
    }
    
    return true;
}

/**
 * @brief Print help message
 */
void printHelp() {
    std::cout << "\n=== ZED Video Extractor CLI ===\n\n";
    std::cout << "Extract video from ZED SVO2 files.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  video_extractor_cli <svo_file> [options]\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  <svo_file>              Path to SVO2 file\n\n";
    std::cout << "Options:\n";
    std::cout << "  --base-output <path>    Base output directory\n";
    std::cout << "                          (default: E:/Turbulence Solutions/AeroLock/ZED_Recordings_Output)\n";
    std::cout << "  --camera <mode>         Camera mode:\n";
    std::cout << "                            left          - Left camera only\n";
    std::cout << "                            right         - Right camera only\n";
    std::cout << "                            both_separate - Two separate videos\n";
    std::cout << "                            side_by_side  - Stereo side-by-side\n";
    std::cout << "                          (default: left)\n";
    std::cout << "  --codec <codec>         Video codec: h264, h265 (default: h264)\n";
    std::cout << "  --fps <rate>            Output FPS (default: source FPS)\n";
    std::cout << "  --quality <0-100>       Video quality (default: 90)\n";
    std::cout << "  --help, -h              Show this help message\n\n";
    std::cout << "Output Structure:\n";
    std::cout << "  Videos saved to: <base>/Extractions/flight_XXX/extraction_NNN/\n";
    std::cout << "  Each extraction gets a unique numbered folder\n\n";
    std::cout << "Examples:\n";
    std::cout << "  video_extractor_cli flight.svo2\n";
    std::cout << "  video_extractor_cli flight.svo2 --camera side_by_side --codec h265\n";
    std::cout << "  video_extractor_cli flight.svo2 --fps 30 --quality 95\n\n";
}

/**
 * @brief Validate configuration
 */
ErrorResult validateConfig(const Config& config) {
    // Check SVO file exists
    if (!FileUtils::validateSVO2File(config.svoFilePath)) {
        return ErrorResult::failure("Invalid SVO2 file: " + config.svoFilePath);
    }
    
    // Check camera mode
    if (config.cameraMode != "left" && config.cameraMode != "right" && 
        config.cameraMode != "both_separate" && config.cameraMode != "side_by_side") {
        return ErrorResult::failure("Invalid camera mode: " + config.cameraMode);
    }
    
    // Check codec
    if (config.codec != "h264" && config.codec != "h265") {
        return ErrorResult::failure("Invalid codec: " + config.codec);
    }
    
    // Check FPS
    if (config.outputFps != -1.0f && config.outputFps <= 0) {
        return ErrorResult::failure("FPS must be positive: " + std::to_string(config.outputFps));
    }
    
    // Check quality
    if (config.quality < 0 || config.quality > 100) {
        return ErrorResult::failure("Quality must be 0-100: " + std::to_string(config.quality));
    }
    
    return ErrorResult::success();
}

/**
 * @brief Convert sl::Mat to cv::Mat
 */
cv::Mat slMat2cvMat(sl::Mat& input) {
    int cvType;
    switch (input.getDataType()) {
        case sl::MAT_TYPE::F32_C1: cvType = CV_32FC1; break;
        case sl::MAT_TYPE::F32_C2: cvType = CV_32FC2; break;
        case sl::MAT_TYPE::F32_C3: cvType = CV_32FC3; break;
        case sl::MAT_TYPE::F32_C4: cvType = CV_32FC4; break;
        case sl::MAT_TYPE::U8_C1: cvType = CV_8UC1; break;
        case sl::MAT_TYPE::U8_C2: cvType = CV_8UC2; break;
        case sl::MAT_TYPE::U8_C3: cvType = CV_8UC3; break;
        case sl::MAT_TYPE::U8_C4: cvType = CV_8UC4; break;
        default: cvType = CV_8UC1; break;
    }
    
    return cv::Mat(input.getHeight(), input.getWidth(), cvType, input.getPtr<sl::uchar1>());
}

/**
 * @brief Get OpenCV video codec fourcc
 */
int getVideoCodec(const std::string& codec) {
    if (codec == "h264") {
        // H.264 codec - requires OpenCV with proper FFmpeg support
        return cv::VideoWriter::fourcc('H', '2', '6', '4');
    } else if (codec == "h265") {
        // H.265 codec
        return cv::VideoWriter::fourcc('H', 'E', 'V', 'C');
    } else if (codec == "mjpeg") {
        return cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    }
    return cv::VideoWriter::fourcc('H', '2', '6', '4'); // Default to H.264
}

/**
 * @brief Extract video from SVO file
 */
ErrorResult extractVideo(const Config& config) {
    LOG_INFO("Starting video extraction...");
    LOG_INFO("Input: " + config.svoFilePath);
    LOG_INFO("Base output: " + config.baseOutputPath);
    LOG_INFO("Camera: " + config.cameraMode);
    LOG_INFO("Codec: " + config.codec);
    LOG_INFO("Quality: " + std::to_string(config.quality));
    
    // Initialize OutputManager
    OutputManager outputMgr(config.baseOutputPath);
    auto validateResult = outputMgr.validateBaseOutputPath();
    if (validateResult.isFailure()) {
        return validateResult;
    }
    
    // Open SVO file
    SVOHandler svo(config.svoFilePath);
    if (!svo.open()) {
        return ErrorResult::failure("Failed to open SVO file: " + svo.getLastError());
    }
    
    // Get SVO properties
    SVOProperties props = svo.getProperties();
    LOG_INFO("SVO Properties:");
    LOG_INFO("  Camera: " + props.cameraModel);
    LOG_INFO("  Resolution: " + std::to_string(props.width) + "x" + std::to_string(props.height));
    LOG_INFO("  FPS: " + std::to_string(props.fps));
    LOG_INFO("  Total Frames: " + std::to_string(props.totalFrames));
    LOG_INFO("  Duration: " + std::to_string(props.durationSeconds) + "s");
    
    // Parse flight info
    std::filesystem::path svoPath(config.svoFilePath);
    std::string parentFolder = svoPath.parent_path().filename().string();
    std::string flightFolderName = parentFolder;
    
    if (!FileUtils::isFlightFolder(parentFolder)) {
        LOG_WARNING("SVO file not in flight folder format. Using filename as identifier.");
        flightFolderName = svoPath.stem().string();
    }
    
    // Get extraction output path
    std::string extractionPath = outputMgr.getExtractionPath(flightFolderName, OutputType::VIDEO);
    if (extractionPath.empty()) {
        return ErrorResult::failure("Failed to create extraction directory");
    }
    
    LOG_INFO("Extraction path: " + extractionPath);
    
    // Determine output FPS with validation
    float outputFps;
    if (config.outputFps > 0) {
        outputFps = config.outputFps;
        // Warn if requested FPS exceeds source FPS
        if (outputFps > props.fps) {
            LOG_WARNING("Requested FPS (" + std::to_string(outputFps) + 
                       ") exceeds source FPS (" + std::to_string(props.fps) + 
                       "), falling back to source FPS");
            outputFps = props.fps;
        }
    } else {
        outputFps = props.fps;  // Use source FPS when 0
    }
    LOG_INFO("Output FPS: " + std::to_string(outputFps));
    
    // Prepare metadata
    VideoMetadata videoMeta;
    videoMeta.extractionDateTime = getCurrentDateTime();
    videoMeta.width = props.width;
    videoMeta.height = props.height;
    videoMeta.fps = outputFps;
    videoMeta.totalFrames = props.totalFrames;
    videoMeta.durationSeconds = props.durationSeconds;
    videoMeta.cameraMode = config.cameraMode;
    videoMeta.videoCodec = config.codec;
    videoMeta.outputFormat = "mp4";
    
    if (FileUtils::isFlightFolder(parentFolder)) {
        FlightInfo flightInfo;
        if (flightInfo.parseFromFolder(svoPath.parent_path().string())) {
            videoMeta.flightInfo = flightInfo;
        }
    }
    
    // Setup video codec
    int fourcc = getVideoCodec(config.codec);
    cv::Size frameSize(props.width, props.height);
    
    // For side-by-side, double the width
    if (config.cameraMode == "side_by_side") {
        frameSize.width *= 2;
    }
    
    // Create video writers
    cv::VideoWriter leftWriter, rightWriter, stereoWriter;
    std::string baseFilename = "video";
    
    if (config.cameraMode == "left" || config.cameraMode == "both_separate") {
        std::string leftPath = extractionPath + "/" + baseFilename + "_left.mp4";
        leftWriter.open(leftPath, fourcc, outputFps, cv::Size(props.width, props.height), true);
        if (!leftWriter.isOpened()) {
            return ErrorResult::failure("Failed to create left video writer: " + leftPath);
        }
        videoMeta.outputFiles.push_back(leftPath);
        LOG_INFO("Created left video: " + leftPath);
    }
    
    if (config.cameraMode == "right" || config.cameraMode == "both_separate") {
        std::string rightPath = extractionPath + "/" + baseFilename + "_right.mp4";
        rightWriter.open(rightPath, fourcc, outputFps, cv::Size(props.width, props.height), true);
        if (!rightWriter.isOpened()) {
            return ErrorResult::failure("Failed to create right video writer: " + rightPath);
        }
        videoMeta.outputFiles.push_back(rightPath);
        LOG_INFO("Created right video: " + rightPath);
    }
    
    if (config.cameraMode == "side_by_side") {
        std::string stereoPath = extractionPath + "/" + baseFilename + "_stereo.mp4";
        stereoWriter.open(stereoPath, fourcc, outputFps, frameSize, true);
        if (!stereoWriter.isOpened()) {
            return ErrorResult::failure("Failed to create stereo video writer: " + stereoPath);
        }
        videoMeta.outputFiles.push_back(stereoPath);
        LOG_INFO("Created stereo video: " + stereoPath);
    }
    
    // Extraction loop
    sl::Mat leftImage, rightImage;
    int frameCount = 0;
    int progressInterval = props.totalFrames / 20; // Update every 5%
    if (progressInterval < 1) progressInterval = 1;
    
    LOG_INFO("Processing frames...");
    
    while (svo.grab()) {
        // Retrieve images
        if (config.cameraMode == "left" || config.cameraMode == "both_separate" || config.cameraMode == "side_by_side") {
            sl::ERROR_CODE err = svo.retrieveImage(leftImage, sl::VIEW::LEFT);
            if (err != sl::ERROR_CODE::SUCCESS) {
                LOG_WARNING("Failed to retrieve left image at frame " + std::to_string(frameCount));
                frameCount++;
                continue;
            }
        }
        
        if (config.cameraMode == "right" || config.cameraMode == "both_separate" || config.cameraMode == "side_by_side") {
            sl::ERROR_CODE err = svo.retrieveImage(rightImage, sl::VIEW::RIGHT);
            if (err != sl::ERROR_CODE::SUCCESS) {
                LOG_WARNING("Failed to retrieve right image at frame " + std::to_string(frameCount));
                frameCount++;
                continue;
            }
        }
        
        // Convert to OpenCV Mat
        cv::Mat cvLeft, cvRight;
        if (config.cameraMode == "left" || config.cameraMode == "both_separate" || config.cameraMode == "side_by_side") {
            cvLeft = slMat2cvMat(leftImage);
            cv::cvtColor(cvLeft, cvLeft, cv::COLOR_BGRA2BGR); // ZED returns BGRA
        }
        
        if (config.cameraMode == "right" || config.cameraMode == "both_separate" || config.cameraMode == "side_by_side") {
            cvRight = slMat2cvMat(rightImage);
            cv::cvtColor(cvRight, cvRight, cv::COLOR_BGRA2BGR);
        }
        
        // Write frames
        if (config.cameraMode == "left") {
            leftWriter.write(cvLeft);
        }
        else if (config.cameraMode == "right") {
            rightWriter.write(cvRight);
        }
        else if (config.cameraMode == "both_separate") {
            leftWriter.write(cvLeft);
            rightWriter.write(cvRight);
        }
        else if (config.cameraMode == "side_by_side") {
            // Create side-by-side frame
            cv::Mat stereoFrame(props.height, props.width * 2, CV_8UC3);
            cvLeft.copyTo(stereoFrame(cv::Rect(0, 0, props.width, props.height)));
            cvRight.copyTo(stereoFrame(cv::Rect(props.width, 0, props.width, props.height)));
            stereoWriter.write(stereoFrame);
        }
        
        frameCount++;
        
        // Progress update
        if (frameCount % progressInterval == 0) {
            float progress = (frameCount * 100.0f) / props.totalFrames;
            LOG_INFO("Progress: " + std::to_string(static_cast<int>(progress)) + 
                    "% (" + std::to_string(frameCount) + "/" + std::to_string(props.totalFrames) + " frames)");
        }
    }
    
    // Release video writers
    if (leftWriter.isOpened()) leftWriter.release();
    if (rightWriter.isOpened()) rightWriter.release();
    if (stereoWriter.isOpened()) stereoWriter.release();
    
    // Save metadata
    std::string metadataPath = OutputManager::getMetadataPath(extractionPath);
    if (!videoMeta.saveToJSON(metadataPath)) {
        LOG_WARNING("Failed to save metadata: " + metadataPath);
    } else {
        LOG_INFO("Metadata saved: " + metadataPath);
    }
    
    LOG_INFO("Video extraction complete!");
    LOG_INFO("Total frames processed: " + std::to_string(frameCount));
    LOG_INFO("Extraction directory: " + extractionPath);
    
    return ErrorResult::success();
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    // Parse arguments first
    Config config;
    if (!parseArguments(argc, argv, config)) {
        printHelp();
        return 1;
    }
    
    if (config.showHelp) {
        printHelp();
        return 0;
    }
    
    // Initialize logger
    try {
        Logger::getInstance().initialize("video_extractor.log", LogMode::BOTH, LogLevel::INFO);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize logger: " << e.what() << std::endl;
        std::cerr << "Continuing without file logging..." << std::endl;
    }
    
    std::cout << "\n=== ZED Video Extractor CLI v0.1.0 ===\n" << std::endl;
    LOG_INFO("ZED Video Extractor CLI v0.1.0 started");
    
    // Validate configuration
    auto validationResult = validateConfig(config);
    if (validationResult.isFailure()) {
        LOG_ERROR(validationResult.getMessage());
        std::cerr << "Error: " << validationResult.getMessage() << std::endl;
        return 1;
    }
    
    // Extract video
    auto result = extractVideo(config);
    if (result.isFailure()) {
        LOG_ERROR(result.getMessage());
        std::cerr << "Error: " << result.getMessage() << std::endl;
        return 1;
    }
    
    std::cout << "\n✓ Video extraction complete!\n" << std::endl;
    LOG_INFO("Application finished successfully");
    Logger::getInstance().shutdown();
    
    return 0;
}
