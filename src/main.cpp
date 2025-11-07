/**
 * @file main.cpp
 * @brief ZED Camera SVO2 File Extraction Tool
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 * @version 1.0.0
 * 
 * @description
 * This application extracts frames from Stereolabs ZED camera SVO2 (Stereo Video Output) files.
 * It processes all SVO2 files in a specified input directory and exports the frames to an output
 * directory with proper organization and metadata.
 * 
 * Key Features:
 * - Batch processing of multiple SVO2 files
 * - Frame extraction with configurable format (PNG, JPEG, etc.)
 * - Progress tracking and error handling
 * - Metadata preservation
 * - Left and Right camera image extraction
 * - Depth map extraction (optional)
 * 
 * Dependencies:
 * - ZED SDK 4.x (Stereolabs)
 * - OpenCV (for image processing and saving)
 * - C++17 or later
 * 
 * Input Directory:  E:\Turbulence Solutions\AeroLock\ZED_Recordings
 * Output Directory: E:\Turbulence Solutions\AeroLock\ZED_Recordings_Output
 */

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <sstream>

namespace fs = std::filesystem;

/**
 * @brief Configuration structure for the extraction process
 * 
 * This structure holds all configurable parameters for the SVO2 extraction process.
 * Modify these values to customize the extraction behavior.
 */
struct ExtractionConfig {
    std::string inputFolder;        ///< Directory containing SVO2 files
    std::string outputFolder;       ///< Directory where extracted frames will be saved
    std::string imageFormat;        ///< Output image format (e.g., "png", "jpg")
    bool extractDepth;              ///< Whether to extract depth maps
    bool extractLeftImage;          ///< Whether to extract left camera images
    bool extractRightImage;         ///< Whether to extract right camera images
    int frameSkip;                  ///< Extract every Nth frame (1 = all frames)
    
    /**
     * @brief Default constructor with standard configuration
     */
    ExtractionConfig() 
        : inputFolder("E:\\Turbulence Solutions\\AeroLock\\ZED_Recordings"),
          outputFolder("E:\\Turbulence Solutions\\AeroLock\\ZED_Recordings_Output"),
          imageFormat("png"),
          extractDepth(true),
          extractLeftImage(true),
          extractRightImage(true),
          frameSkip(1) {}
};

/**
 * @brief Converts a ZED SDK Mat to an OpenCV Mat for processing
 * 
 * @param input ZED SDK Mat object containing image data
 * @param output OpenCV Mat object to store the converted image
 * @return true if conversion was successful, false otherwise
 */
bool slMat2cvMat(sl::Mat& input, cv::Mat& output) {
    // Get the data type of the input
    int cv_type = -1;
    
    switch (input.getDataType()) {
        case sl::MAT_TYPE::F32_C1:
            cv_type = CV_32FC1;
            break;
        case sl::MAT_TYPE::F32_C2:
            cv_type = CV_32FC2;
            break;
        case sl::MAT_TYPE::F32_C3:
            cv_type = CV_32FC3;
            break;
        case sl::MAT_TYPE::F32_C4:
            cv_type = CV_32FC4;
            break;
        case sl::MAT_TYPE::U8_C1:
            cv_type = CV_8UC1;
            break;
        case sl::MAT_TYPE::U8_C2:
            cv_type = CV_8UC2;
            break;
        case sl::MAT_TYPE::U8_C3:
            cv_type = CV_8UC3;
            break;
        case sl::MAT_TYPE::U8_C4:
            cv_type = CV_8UC4;
            break;
        default:
            return false;
    }
    
    // Create OpenCV Mat from ZED Mat data
    output = cv::Mat(input.getHeight(), input.getWidth(), cv_type, input.getPtr<sl::uchar1>());
    return true;
}

/**
 * @brief Creates the output directory structure for a specific SVO file
 * 
 * Creates subdirectories for left images, right images, and depth maps
 * within the main output folder.
 * 
 * @param outputBase Base output directory path
 * @param svoFileName Name of the SVO2 file being processed
 * @return Path to the created directory for this SVO file
 */
std::string createOutputDirectories(const std::string& outputBase, const std::string& svoFileName) {
    // Remove extension from SVO filename
    std::string baseName = fs::path(svoFileName).stem().string();
    
    // Create main output directory for this SVO file
    fs::path outputPath = fs::path(outputBase) / baseName;
    fs::create_directories(outputPath);
    
    // Create subdirectories for different image types
    fs::create_directories(outputPath / "left");
    fs::create_directories(outputPath / "right");
    fs::create_directories(outputPath / "depth");
    
    std::cout << "[INFO] Created output directories for: " << baseName << std::endl;
    
    return outputPath.string();
}

/**
 * @brief Extracts all frames from a single SVO2 file
 * 
 * Opens the SVO2 file, iterates through all frames, and saves the requested
 * images (left, right, depth) according to the configuration.
 * 
 * @param svoFilePath Full path to the SVO2 file
 * @param config Extraction configuration parameters
 * @return true if extraction was successful, false if errors occurred
 */
bool extractSVO2File(const std::string& svoFilePath, const ExtractionConfig& config) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "[PROCESSING] SVO File: " << fs::path(svoFilePath).filename().string() << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // Initialize ZED Camera object
    sl::Camera zed;
    sl::InitParameters init_params;
    
    // Set the SVO file path
    init_params.input.setFromSVOFile(svoFilePath.c_str());
    init_params.coordinate_units = sl::UNIT::METER;
    init_params.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP;
    
    // Open the SVO file
    sl::ERROR_CODE err = zed.open(init_params);
    if (err != sl::ERROR_CODE::SUCCESS) {
        std::cerr << "[ERROR] Failed to open SVO file: " << sl::toString(err) << std::endl;
        return false;
    }
    
    // Get SVO file properties
    sl::RecordingParameters recordingParams = zed.getRecordingParameters();
    int totalFrames = zed.getSVONumberOfFrames();
    
    std::cout << "[INFO] Total frames in SVO: " << totalFrames << std::endl;
    std::cout << "[INFO] Video resolution: " << zed.getCameraInformation().camera_configuration.resolution.width 
              << "x" << zed.getCameraInformation().camera_configuration.resolution.height << std::endl;
    std::cout << "[INFO] FPS: " << zed.getCameraInformation().camera_configuration.fps << std::endl;
    
    // Create output directory structure
    std::string outputDir = createOutputDirectories(config.outputFolder, 
                                                    fs::path(svoFilePath).filename().string());
    
    // Prepare image containers
    sl::Mat leftImage, rightImage, depthMap;
    cv::Mat cvLeft, cvRight, cvDepth;
    
    int frameCount = 0;
    int savedFrameCount = 0;
    
    // Runtime parameters for frame grabbing
    sl::RuntimeParameters runtime_params;
    runtime_params.sensing_mode = sl::SENSING_MODE::STANDARD;
    
    // Start timing
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process all frames in the SVO file
    while (true) {
        // Grab next frame
        sl::ERROR_CODE grabErr = zed.grab(runtime_params);
        
        if (grabErr == sl::ERROR_CODE::END_OF_SVOFILE_REACHED) {
            std::cout << "[INFO] Reached end of SVO file" << std::endl;
            break;
        }
        
        if (grabErr != sl::ERROR_CODE::SUCCESS) {
            std::cerr << "[WARNING] Frame grab error: " << sl::toString(grabErr) << std::endl;
            continue;
        }
        
        frameCount++;
        
        // Skip frames according to configuration
        if ((frameCount - 1) % config.frameSkip != 0) {
            continue;
        }
        
        try {
            // Extract and save left image
            if (config.extractLeftImage) {
                zed.retrieveImage(leftImage, sl::VIEW::LEFT);
                if (slMat2cvMat(leftImage, cvLeft)) {
                    std::stringstream ss;
                    ss << outputDir << "/left/frame_" << std::setw(6) << std::setfill('0') 
                       << frameCount << "." << config.imageFormat;
                    cv::imwrite(ss.str(), cvLeft);
                }
            }
            
            // Extract and save right image
            if (config.extractRightImage) {
                zed.retrieveImage(rightImage, sl::VIEW::RIGHT);
                if (slMat2cvMat(rightImage, cvRight)) {
                    std::stringstream ss;
                    ss << outputDir << "/right/frame_" << std::setw(6) << std::setfill('0') 
                       << frameCount << "." << config.imageFormat;
                    cv::imwrite(ss.str(), cvRight);
                }
            }
            
            // Extract and save depth map
            if (config.extractDepth) {
                zed.retrieveMeasure(depthMap, sl::MEASURE::DEPTH);
                if (slMat2cvMat(depthMap, cvDepth)) {
                    std::stringstream ss;
                    ss << outputDir << "/depth/frame_" << std::setw(6) << std::setfill('0') 
                       << frameCount << ".png";
                    
                    // Normalize depth for visualization
                    cv::Mat depthNormalized;
                    cv::normalize(cvDepth, depthNormalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                    cv::imwrite(ss.str(), depthNormalized);
                }
            }
            
            savedFrameCount++;
            
            // Display progress every 10 frames
            if (savedFrameCount % 10 == 0) {
                float progress = (float)frameCount / (float)totalFrames * 100.0f;
                std::cout << "[PROGRESS] Extracted " << savedFrameCount << " frames (" 
                         << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Exception while processing frame " << frameCount 
                     << ": " << e.what() << std::endl;
        }
    }
    
    // Calculate processing time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
    
    std::cout << "\n[SUCCESS] Extraction completed!" << std::endl;
    std::cout << "  - Total frames processed: " << frameCount << std::endl;
    std::cout << "  - Frames saved: " << savedFrameCount << std::endl;
    std::cout << "  - Processing time: " << duration.count() << " seconds" << std::endl;
    std::cout << "  - Output location: " << outputDir << std::endl;
    
    // Close the camera
    zed.close();
    
    return true;
}

/**
 * @brief Main entry point of the application
 * 
 * Scans the input directory for SVO2 files and processes each one.
 * Handles command-line arguments and overall execution flow.
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return 0 on success, non-zero on failure
 */
int main(int argc, char** argv) {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        ZED Camera SVO2 Frame Extraction Tool v1.0.0          ║\n";
    std::cout << "║              Created by Angelo Amon (xX2Angelo8Xx)            ║\n";
    std::cout << "║                    November 7, 2025                           ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;
    
    // Initialize configuration
    ExtractionConfig config;
    
    // Optional: Override configuration with command-line arguments
    if (argc > 1) {
        config.inputFolder = argv[1];
        if (argc > 2) {
            config.outputFolder = argv[2];
        }
    }
    
    std::cout << "[CONFIG] Input Directory:  " << config.inputFolder << std::endl;
    std::cout << "[CONFIG] Output Directory: " << config.outputFolder << std::endl;
    std::cout << "[CONFIG] Image Format:     " << config.imageFormat << std::endl;
    std::cout << "[CONFIG] Frame Skip:       Every " << config.frameSkip << " frame(s)" << std::endl;
    std::cout << std::endl;
    
    // Verify input directory exists
    if (!fs::exists(config.inputFolder)) {
        std::cerr << "[ERROR] Input directory does not exist: " << config.inputFolder << std::endl;
        std::cerr << "[INFO] Please create the directory or specify a different path." << std::endl;
        return 1;
    }
    
    // Create output directory if it doesn't exist
    try {
        fs::create_directories(config.outputFolder);
        std::cout << "[INFO] Output directory ready: " << config.outputFolder << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ERROR] Failed to create output directory: " << e.what() << std::endl;
        return 1;
    }
    
    // Scan for SVO2 files in input directory
    std::vector<std::string> svoFiles;
    
    std::cout << "[INFO] Scanning for SVO2 files..." << std::endl;
    
    try {
        for (const auto& entry : fs::directory_iterator(config.inputFolder)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                // Convert to lowercase for case-insensitive comparison
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                
                if (extension == ".svo" || extension == ".svo2") {
                    svoFiles.push_back(entry.path().string());
                    std::cout << "  [FOUND] " << entry.path().filename().string() << std::endl;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ERROR] Failed to scan directory: " << e.what() << std::endl;
        return 1;
    }
    
    // Check if any SVO files were found
    if (svoFiles.empty()) {
        std::cout << "\n[WARNING] No SVO/SVO2 files found in the input directory." << std::endl;
        std::cout << "[INFO] Please add SVO2 files to: " << config.inputFolder << std::endl;
        return 0;
    }
    
    std::cout << "\n[INFO] Found " << svoFiles.size() << " SVO file(s) to process" << std::endl;
    
    // Process each SVO file
    int successCount = 0;
    int failureCount = 0;
    
    auto overallStartTime = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < svoFiles.size(); ++i) {
        std::cout << "\n[" << (i + 1) << "/" << svoFiles.size() << "] ";
        
        if (extractSVO2File(svoFiles[i], config)) {
            successCount++;
        } else {
            failureCount++;
            std::cerr << "[FAILED] Could not extract: " << svoFiles[i] << std::endl;
        }
    }
    
    // Calculate total processing time
    auto overallEndTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::minutes>(overallEndTime - overallStartTime);
    
    // Print final summary
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "                        EXTRACTION SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "  Total files processed:  " << svoFiles.size() << std::endl;
    std::cout << "  Successfully extracted: " << successCount << std::endl;
    std::cout << "  Failed:                 " << failureCount << std::endl;
    std::cout << "  Total processing time:  " << totalDuration.count() << " minutes" << std::endl;
    std::cout << "  Output location:        " << config.outputFolder << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "\n[COMPLETE] All processing finished!" << std::endl;
    
    return (failureCount == 0) ? 0 : 1;
}
