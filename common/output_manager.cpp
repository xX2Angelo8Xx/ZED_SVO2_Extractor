/**
 * @file output_manager.cpp
 * @brief Implementation of OutputManager
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "output_manager.hpp"
#include "error_handler.hpp"
#include "metadata.hpp"
#include <regex>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace zed_tools {

OutputManager::OutputManager(const std::string& baseOutputPath) 
    : baseOutputPath_(baseOutputPath) {
    // Normalize path separators
    std::replace(baseOutputPath_.begin(), baseOutputPath_.end(), '\\', '/');
    
    // Setup standard paths
    extractionsPath_ = baseOutputPath_ + "/Extractions";
    yoloTrainingPath_ = baseOutputPath_ + "/Yolo_Training/Unfiltered_Images";
    frameCounterFile_ = baseOutputPath_ + "/Yolo_Training/.frame_counter.json";
}

ErrorResult OutputManager::validateBaseOutputPath() const {
    if (!fs::exists(baseOutputPath_)) {
        // Try to create it
        try {
            fs::create_directories(baseOutputPath_);
            LOG_INFO("Created base output directory: " + baseOutputPath_);
        } catch (const std::exception& e) {
            return ErrorResult::failure("Failed to create base output directory: " + 
                                      baseOutputPath_ + " - " + e.what());
        }
    }
    
    // Check if writable
    try {
        std::string testFile = baseOutputPath_ + "/.write_test";
        std::ofstream test(testFile);
        if (!test.is_open()) {
            return ErrorResult::failure("Base output directory is not writable: " + baseOutputPath_);
        }
        test.close();
        fs::remove(testFile);
    } catch (const std::exception& e) {
        return ErrorResult::failure("Cannot write to base output directory: " + 
                                  baseOutputPath_ + " - " + e.what());
    }
    
    return ErrorResult::success();
}

bool OutputManager::ensureDirectoryExists(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
            LOG_DEBUG("Created directory: " + path);
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create directory: " + path + " - " + e.what());
        return false;
    }
}

int OutputManager::getNextExtractionNumber(const std::string& flightFolderName) {
    std::string flightPath = extractionsPath_ + "/" + flightFolderName;
    
    if (!fs::exists(flightPath)) {
        return 1; // First extraction
    }
    
    int maxNumber = 0;
    std::regex pattern(R"(extraction_(\d{3}))");
    
    try {
        for (const auto& entry : fs::directory_iterator(flightPath)) {
            if (entry.is_directory()) {
                std::string folderName = entry.path().filename().string();
                std::smatch matches;
                if (std::regex_match(folderName, matches, pattern)) {
                    int number = std::stoi(matches[1].str());
                    if (number > maxNumber) {
                        maxNumber = number;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_WARNING("Error scanning extraction folders: " + std::string(e.what()));
    }
    
    return maxNumber + 1;
}

std::string OutputManager::getExtractionPath(const std::string& flightFolderName, OutputType type) {
    // Get next extraction number
    int extractionNum = getNextExtractionNumber(flightFolderName);
    
    // Format extraction folder name: extraction_001, extraction_002, etc.
    std::ostringstream oss;
    oss << "extraction_" << std::setw(3) << std::setfill('0') << extractionNum;
    std::string extractionFolder = oss.str();
    
    // Build full path
    std::string fullPath = extractionsPath_ + "/" + flightFolderName + "/" + extractionFolder;
    
    // Create directory structure
    if (!ensureDirectoryExists(fullPath)) {
        LOG_ERROR("Failed to create extraction directory: " + fullPath);
        return "";
    }
    
    LOG_INFO("Created extraction path: " + fullPath);
    return fullPath;
}

std::string OutputManager::getYoloFramesPath(const std::string& flightFolderName) {
    std::string fullPath = yoloTrainingPath_ + "/" + flightFolderName;
    
    // Create directory structure
    if (!ensureDirectoryExists(fullPath)) {
        LOG_ERROR("Failed to create YOLO frames directory: " + fullPath);
        return "";
    }
    
    LOG_INFO("YOLO frames path: " + fullPath);
    return fullPath;
}

int OutputManager::scanForHighestFrameNumber(const std::string& folderPath) {
    int maxFrameNum = 0;
    std::regex pattern(R"(frame_(\d{5})_)"); // Matches frame_00001_, frame_00002_, etc.
    
    try {
        if (!fs::exists(folderPath)) {
            return 0;
        }
        
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::smatch matches;
                if (std::regex_search(filename, matches, pattern)) {
                    int frameNum = std::stoi(matches[1].str());
                    if (frameNum > maxFrameNum) {
                        maxFrameNum = frameNum;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_WARNING("Error scanning frames in " + folderPath + ": " + e.what());
    }
    
    return maxFrameNum;
}

int OutputManager::getNextGlobalFrameNumber() {
    int maxFrameNum = 0;
    
    // Scan all flight folders in YOLO training directory
    try {
        if (fs::exists(yoloTrainingPath_)) {
            for (const auto& entry : fs::directory_iterator(yoloTrainingPath_)) {
                if (entry.is_directory()) {
                    int folderMax = scanForHighestFrameNumber(entry.path().string());
                    if (folderMax > maxFrameNum) {
                        maxFrameNum = folderMax;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_WARNING("Error scanning YOLO training folders: " + std::string(e.what()));
    }
    
    // Also check frame counter file for consistency
    try {
        if (fs::exists(frameCounterFile_)) {
            std::ifstream file(frameCounterFile_);
            if (file.is_open()) {
                std::string line;
                std::getline(file, line);
                // Simple JSON: {"last_frame": 123}
                std::regex counterPattern(R"("last_frame"\s*:\s*(\d+))");
                std::smatch matches;
                if (std::regex_search(line, matches, counterPattern)) {
                    int counterValue = std::stoi(matches[1].str());
                    if (counterValue > maxFrameNum) {
                        maxFrameNum = counterValue;
                    }
                }
                file.close();
            }
        }
    } catch (const std::exception& e) {
        LOG_WARNING("Error reading frame counter file: " + std::string(e.what()));
    }
    
    LOG_INFO("Next global frame number: " + std::to_string(maxFrameNum + 1));
    return maxFrameNum + 1;
}

void OutputManager::updateGlobalFrameCounter(int lastFrameNumber) {
    try {
        // Ensure directory exists
        std::string counterDir = baseOutputPath_ + "/Yolo_Training";
        ensureDirectoryExists(counterDir);
        
        // Write counter file
        std::ofstream file(frameCounterFile_);
        if (file.is_open()) {
            file << "{\n";
            file << "  \"last_frame\": " << lastFrameNumber << ",\n";
            file << "  \"updated\": \"" << zed_tools::getCurrentDateTime() << "\"\n";
            file << "}\n";
            file.close();
            LOG_DEBUG("Updated global frame counter to: " + std::to_string(lastFrameNumber));
        } else {
            LOG_WARNING("Failed to write frame counter file: " + frameCounterFile_);
        }
    } catch (const std::exception& e) {
        LOG_WARNING("Error updating frame counter: " + std::string(e.what()));
    }
}

std::string OutputManager::getMetadataPath(const std::string& extractionPath) {
    return extractionPath + "/metadata.json";
}

} // namespace zed_tools
