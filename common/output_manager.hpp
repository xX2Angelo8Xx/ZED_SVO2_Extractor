/**
 * @file output_manager.hpp
 * @brief Smart output path management for ZED extraction tools
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 * 
 * Manages output folder structure:
 * - Extractions/flight_XXX/extraction_NNN/ for videos and depth
 * - Yolo_Training/Unfiltered_Images/flight_XXX/ for frames with global numbering
 */

#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include "error_handler.hpp"

namespace zed_tools {

/**
 * @brief Output type for different extraction modes
 */
enum class OutputType {
    VIDEO,      ///< Video extraction (mp4 files)
    FRAMES,     ///< Frame extraction for YOLO training
    DEPTH       ///< Depth analysis heatmap
};

/**
 * @brief Manages intelligent output path generation and organization
 */
class OutputManager {
public:
    /**
     * @brief Constructor
     * @param baseOutputPath Base output directory (e.g., "E:/Turbulence Solutions/AeroLock/ZED_Recordings_Output")
     */
    explicit OutputManager(const std::string& baseOutputPath);
    
    /**
     * @brief Get output path for video/depth extraction
     * @param flightFolderName Flight folder name (e.g., "flight_20251105_205224")
     * @param type Output type (VIDEO or DEPTH)
     * @return Path to extraction folder (auto-incremented)
     * 
     * Returns: baseOutputPath/Extractions/flight_XXX/extraction_NNN/
     */
    std::string getExtractionPath(const std::string& flightFolderName, OutputType type);
    
    /**
     * @brief Get output path for YOLO frame extraction
     * @param flightFolderName Flight folder name
     * @return Path to YOLO frames folder
     * 
     * Returns: baseOutputPath/Yolo_Training/Unfiltered_Images/flight_XXX/
     */
    std::string getYoloFramesPath(const std::string& flightFolderName);
    
    /**
     * @brief Get next global frame number for YOLO training
     * @return Next available frame number (continues from highest existing)
     * 
     * Scans all YOLO training folders to find highest frame number used,
     * then returns next available number for continuous numbering.
     */
    int getNextGlobalFrameNumber();
    
    /**
     * @brief Update global frame counter after extraction
     * @param lastFrameNumber Last frame number used in this extraction
     */
    void updateGlobalFrameCounter(int lastFrameNumber);
    
    /**
     * @brief Get next extraction number for a flight
     * @param flightFolderName Flight folder name
     * @return Next extraction number (e.g., 1, 2, 3...)
     */
    int getNextExtractionNumber(const std::string& flightFolderName);
    
    /**
     * @brief Check if base output directory exists and is writable
     * @return ErrorResult indicating success or failure
     */
    ErrorResult validateBaseOutputPath() const;
    
    /**
     * @brief Get full metadata path for an extraction
     * @param extractionPath Path to extraction folder
     * @return Path to metadata.json file
     */
    static std::string getMetadataPath(const std::string& extractionPath);

private:
    std::string baseOutputPath_;
    std::string extractionsPath_;
    std::string yoloTrainingPath_;
    std::string frameCounterFile_;
    
    /**
     * @brief Scan folder for highest frame number
     * @param folderPath Path to scan
     * @return Highest frame number found, or 0 if none
     */
    int scanForHighestFrameNumber(const std::string& folderPath);
    
    /**
     * @brief Create directory structure if it doesn't exist
     * @param path Path to create
     * @return true if successful or already exists
     */
    bool ensureDirectoryExists(const std::string& path);
};

} // namespace zed_tools
