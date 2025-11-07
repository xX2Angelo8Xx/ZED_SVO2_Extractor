/**
 * @file file_utils.hpp
 * @brief File system utilities for ZED SVO2 processing
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 * 
 * Provides cross-platform file system utilities for:
 * - Scanning directories for SVO2 files
 * - Validating SVO2 file format
 * - Detecting flight folders (flight_YYYYMMDD_HHMMSS pattern)
 * - File existence and size checks
 */

#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace zed_tools {

// Type alias for convenience
namespace fs = std::filesystem;

/**
 * @brief Structure to hold SVO2 file information
 */
struct SVO2FileInfo {
    fs::path filePath;              ///< Full path to the SVO2 file
    std::string fileName;           ///< File name only
    std::string parentFolder;       ///< Parent folder name
    uintmax_t fileSizeBytes;        ///< File size in bytes
    bool isValidFlightFolder;       ///< True if parent folder matches flight_YYYYMMDD_HHMMSS pattern
    
    /**
     * @brief Get human-readable file size string
     * @return Formatted size (e.g., "1.2 GB", "512 MB")
     */
    std::string getFormattedSize() const;
};

/**
 * @namespace FileUtils
 * @brief Collection of file system utility functions
 */
namespace FileUtils {

    /**
     * @brief Scan a directory recursively for SVO2 files
     * @param directoryPath Path to scan
     * @param recursive If true, scan subdirectories
     * @return Vector of SVO2FileInfo structures
     * @throws std::filesystem::filesystem_error on access issues
     * 
     * Example usage:
     * @code
     * auto files = FileUtils::scanForSVO2Files("C:/Data/Flights", true);
     * for (const auto& file : files) {
     *     std::cout << "Found: " << file.fileName << std::endl;
     * }
     * @endcode
     */
    std::vector<SVO2FileInfo> scanForSVO2Files(
        const std::string& directoryPath,
        bool recursive = true
    );

    /**
     * @brief Validate if a file is a valid SVO2 file
     * @param filePath Path to the file to validate
     * @return true if file exists, has .svo2 extension, and is not empty
     * 
     * Validation checks:
     * 1. File exists
     * 2. Has .svo2 extension (case-insensitive)
     * 3. File size > 0 bytes
     * 4. File is readable
     */
    bool validateSVO2File(const fs::path& filePath);

    /**
     * @brief Check if a folder name matches the flight folder pattern
     * @param folderName Name of the folder (not full path)
     * @return true if matches flight_YYYYMMDD_HHMMSS pattern
     * 
     * Valid examples:
     * - flight_20251107_143056
     * - flight_20240315_090000
     */
    bool isFlightFolder(const std::string& folderName);

    /**
     * @brief Get all flight folders in a directory
     * @param directoryPath Path to scan
     * @param recursive If true, scan subdirectories
     * @return Vector of paths to flight folders
     * 
     * Only returns folders matching the flight_YYYYMMDD_HHMMSS pattern
     */
    std::vector<fs::path> getFlightFolders(
        const std::string& directoryPath,
        bool recursive = false
    );

    /**
     * @brief Check if a file exists
     * @param filePath Path to check
     * @return true if file exists and is a regular file
     */
    bool fileExists(const fs::path& filePath);

    /**
     * @brief Check if a directory exists
     * @param directoryPath Path to check
     * @return true if directory exists and is a directory
     */
    bool directoryExists(const fs::path& directoryPath);

    /**
     * @brief Get file size in bytes
     * @param filePath Path to the file
     * @return File size in bytes, or 0 if file doesn't exist
     */
    uintmax_t getFileSize(const fs::path& filePath);

    /**
     * @brief Format file size into human-readable string
     * @param bytes Size in bytes
     * @return Formatted string (e.g., "1.2 GB", "512 MB", "4.5 KB")
     */
    std::string formatFileSize(uintmax_t bytes);

    /**
     * @brief Create directory (including parent directories if needed)
     * @param directoryPath Path to create
     * @return true if directory was created or already exists
     * @throws std::filesystem::filesystem_error on permission issues
     */
    bool createDirectory(const fs::path& directoryPath);

    /**
     * @brief Get absolute path from relative path
     * @param relativePath Relative path
     * @return Absolute path
     * @throws std::filesystem::filesystem_error on invalid path
     */
    fs::path getAbsolutePath(const fs::path& relativePath);

    /**
     * @brief Extract file extension (lowercase)
     * @param filePath Path to the file
     * @return Extension without dot (e.g., "svo2", "mp4")
     */
    std::string getExtension(const fs::path& filePath);

    /**
     * @brief Get filename without extension
     * @param filePath Path to the file
     * @return Filename stem (e.g., "video" from "video.mp4")
     */
    std::string getStem(const fs::path& filePath);

    /**
     * @brief Sanitize filename by removing invalid characters
     * @param filename Filename to sanitize
     * @return Sanitized filename safe for Windows/Unix
     * 
     * Removes: < > : " / \ | ? *
     * Replaces with underscore
     */
    std::string sanitizeFilename(const std::string& filename);

} // namespace FileUtils

} // namespace zed_tools
