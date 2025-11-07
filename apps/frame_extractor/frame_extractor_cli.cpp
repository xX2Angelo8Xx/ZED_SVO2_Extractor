/**
 * @file frame_extractor_cli.cpp
 * @brief Command-line frame extractor for ZED SVO2 files
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 * 
 * Extracts frames from SVO2 files at specified frame rate (default 1fps)
 * for YOLO training data preparation.
 * 
 * Usage:
 *   frame_extractor_cli <svo_file> [options]
 * 
 * Options:
 *   --output-dir <path>     Output directory (default: ./extracted_frames)
 *   --fps <rate>            Extraction frame rate (default: 1.0)
 *   --camera <mode>         Camera mode: left, right, both (default: left)
 *   --format <ext>          Output format: png, jpg (default: png)
 *   --help                  Show this help message
 */

#include <iostream>
#include <string>
#include <cmath>
#include <sl/Camera.hpp>

// Our common utilities
#include "../../common/error_handler.hpp"
#include "../../common/file_utils.hpp"
#include "../../common/svo_handler.hpp"
#include "../../common/metadata.hpp"

using namespace zed_tools;

/**
 * @brief Application configuration
 */
struct Config {
    std::string svoFilePath;
    std::string outputDir = "./extracted_frames";
    float extractionFps = 1.0f;
    std::string cameraMode = "left";  // left, right, both
    std::string outputFormat = "png";
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
        
        if (arg == "--output-dir" && i + 1 < argc) {
            config.outputDir = argv[++i];
        }
        else if (arg == "--fps" && i + 1 < argc) {
            config.extractionFps = std::stof(argv[++i]);
        }
        else if (arg == "--camera" && i + 1 < argc) {
            config.cameraMode = argv[++i];
        }
        else if (arg == "--format" && i + 1 < argc) {
            config.outputFormat = argv[++i];
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
    std::cout << "\n=== ZED Frame Extractor CLI ===\n\n";
    std::cout << "Extract frames from ZED SVO2 files for YOLO training.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  frame_extractor_cli <svo_file> [options]\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  <svo_file>              Path to SVO2 file\n\n";
    std::cout << "Options:\n";
    std::cout << "  --output-dir <path>     Output directory (default: ./extracted_frames)\n";
    std::cout << "  --fps <rate>            Extraction frame rate (default: 1.0)\n";
    std::cout << "  --camera <mode>         Camera: left, right, both (default: left)\n";
    std::cout << "  --format <ext>          Format: png, jpg (default: png)\n";
    std::cout << "  --help, -h              Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  frame_extractor_cli flight.svo2\n";
    std::cout << "  frame_extractor_cli flight.svo2 --fps 2.0 --camera both\n";
    std::cout << "  frame_extractor_cli flight.svo2 --output-dir ./frames --format jpg\n\n";
}

/**
 * @brief Validate configuration
 */
ErrorResult validateConfig(const Config& config) {
    // Check SVO file exists
    if (!FileUtils::validateSVO2File(config.svoFilePath)) {
        return ErrorResult::failure("Invalid SVO2 file: " + config.svoFilePath);
    }
    
    // Check FPS is positive
    if (config.extractionFps <= 0) {
        return ErrorResult::failure("FPS must be positive: " + std::to_string(config.extractionFps));
    }
    
    // Check camera mode
    if (config.cameraMode != "left" && config.cameraMode != "right" && config.cameraMode != "both") {
        return ErrorResult::failure("Invalid camera mode: " + config.cameraMode);
    }
    
    // Check output format
    if (config.outputFormat != "png" && config.outputFormat != "jpg" && config.outputFormat != "jpeg") {
        return ErrorResult::failure("Invalid output format: " + config.outputFormat);
    }
    
    return ErrorResult::success();
}

/**
 * @brief Extract frames from SVO file
 */
ErrorResult extractFrames(const Config& config) {
    LOG_INFO("Starting frame extraction...");
    LOG_INFO("Input: " + config.svoFilePath);
    LOG_INFO("Output: " + config.outputDir);
    LOG_INFO("FPS: " + std::to_string(config.extractionFps));
    LOG_INFO("Camera: " + config.cameraMode);
    LOG_INFO("Format: " + config.outputFormat);
    
    // Create output directory
    if (!FileUtils::createDirectory(config.outputDir)) {
        return ErrorResult::failure("Failed to create output directory: " + config.outputDir);
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
    
    // Calculate frame skip interval
    int frameSkip = static_cast<int>(std::round(props.fps / config.extractionFps));
    if (frameSkip < 1) frameSkip = 1;
    
    LOG_INFO("Extracting every " + std::to_string(frameSkip) + " frames");
    
    // Prepare metadata
    FrameMetadata frameMeta;
    frameMeta.extractionDateTime = metadata_utils::getCurrentDateTime();
    
    // Parse flight info if in flight folder
    std::filesystem::path svoPath(config.svoFilePath);
    std::string parentFolder = svoPath.parent_path().filename().string();
    if (FileUtils::isFlightFolder(parentFolder)) {
        FlightInfo flightInfo;
        if (flightInfo.parseFromFolder(svoPath.parent_path().string())) {
            frameMeta.flightInfo = flightInfo;
        }
    }
    
    // Set metadata
    frameMeta.width = props.width;
    frameMeta.height = props.height;
    frameMeta.sourceFps = props.fps;
    frameMeta.totalSourceFrames = props.totalFrames;
    frameMeta.cameraMode = config.cameraMode;
    frameMeta.imageFormat = config.outputFormat;
    frameMeta.extractionRate = static_cast<int>(config.extractionFps);
    frameMeta.frameSkip = frameSkip;
    frameMeta.outputDirectory = config.outputDir;
    frameMeta.startingFrameNumber = 0;
    frameMeta.totalExtractedFrames = 0;
    
    // Extraction loop
    sl::Mat leftImage, rightImage;
    int frameCount = 0;
    int extractedCount = 0;
    
    while (svo.grab()) {
        // Check if we should extract this frame
        if (frameCount % frameSkip != 0) {
            frameCount++;
            continue;
        }
        
        // Extract left camera
        if (config.cameraMode == "left" || config.cameraMode == "both") {
            sl::ERROR_CODE err = svo.retrieveImage(leftImage, sl::VIEW::LEFT);
            if (err == sl::ERROR_CODE::SUCCESS) {
                // Generate filename
                std::string filename = "frame_" + 
                                     std::to_string(frameCount) + 
                                     "_left." + config.outputFormat;
                std::string filepath = config.outputDir + "/" + filename;
                
                // Save image
                sl::ERROR_CODE saveErr = leftImage.write(filepath.c_str());
                if (saveErr != sl::ERROR_CODE::SUCCESS) {
                    LOG_WARNING("Failed to save frame " + std::to_string(frameCount) + " (left)");
                } else {
                    LOG_DEBUG("Saved: " + filename);
                    extractedCount++;
                }
            }
        }
        
        // Extract right camera
        if (config.cameraMode == "right" || config.cameraMode == "both") {
            sl::ERROR_CODE err = svo.retrieveImage(rightImage, sl::VIEW::RIGHT);
            if (err == sl::ERROR_CODE::SUCCESS) {
                // Generate filename
                std::string filename = "frame_" + 
                                     std::to_string(frameCount) + 
                                     "_right." + config.outputFormat;
                std::string filepath = config.outputDir + "/" + filename;
                
                // Save image
                sl::ERROR_CODE saveErr = rightImage.write(filepath.c_str());
                if (saveErr != sl::ERROR_CODE::SUCCESS) {
                    LOG_WARNING("Failed to save frame " + std::to_string(frameCount) + " (right)");
                } else {
                    LOG_DEBUG("Saved: " + filename);
                    extractedCount++;
                }
            }
        }
        
        frameCount++;
        
        // Progress update every 10 frames
        if (extractedCount % 10 == 0 && extractedCount > 0) {
            float progress = (frameCount * 100.0f) / props.totalFrames;
            LOG_INFO("Progress: " + std::to_string(static_cast<int>(progress)) + 
                    "% (" + std::to_string(extractedCount) + " frames extracted)");
        }
    }
    
    // Update final metadata
    frameMeta.totalExtractedFrames = extractedCount;
    frameMeta.endingFrameNumber = frameCount - 1;
    
    // Save metadata JSON
    std::string metadataPath = config.outputDir + "/frames_metadata.json";
    std::vector<FrameMetadata> metadataList = {frameMeta};
    if (!metadata_utils::saveFrameMetadataList(metadataList, metadataPath)) {
        LOG_WARNING("Failed to save metadata: " + metadataPath);
    } else {
        LOG_INFO("Metadata saved: " + metadataPath);
    }
    
    LOG_INFO("Extraction complete!");
    LOG_INFO("Total frames processed: " + std::to_string(frameCount));
    LOG_INFO("Frames extracted: " + std::to_string(extractedCount));
    LOG_INFO("Output directory: " + config.outputDir);
    
    return ErrorResult::success();
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    // Parse arguments first (before logger initialization)
    Config config;
    if (!parseArguments(argc, argv, config)) {
        printHelp();
        return 1;
    }
    
    if (config.showHelp) {
        printHelp();
        return 0;
    }
    
    // Initialize logger after we know we're not just showing help
    try {
        Logger::getInstance().initialize("frame_extractor.log", LogMode::BOTH, LogLevel::INFO);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize logger: " << e.what() << std::endl;
        std::cerr << "Continuing without file logging..." << std::endl;
    }
    
    std::cout << "\n=== ZED Frame Extractor CLI v0.1.0 ===\n" << std::endl;
    LOG_INFO("ZED Frame Extractor CLI v0.1.0 started");
    
    // Validate configuration
    auto validationResult = validateConfig(config);
    if (validationResult.isFailure()) {
        LOG_ERROR(validationResult.getMessage());
        std::cerr << "Error: " << validationResult.getMessage() << std::endl;
        return 1;
    }
    
    // Extract frames
    auto result = extractFrames(config);
    if (result.isFailure()) {
        LOG_ERROR(result.getMessage());
        std::cerr << "Error: " << result.getMessage() << std::endl;
        return 1;
    }
    
    std::cout << "\n✓ Extraction complete!\n" << std::endl;
    LOG_INFO("Application finished successfully");
    Logger::getInstance().shutdown();
    
    return 0;
}
