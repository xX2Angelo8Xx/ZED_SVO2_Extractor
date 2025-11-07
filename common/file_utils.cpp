/**
 * @file file_utils.cpp
 * @brief Implementation of file system utilities
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "file_utils.hpp"
#include <regex>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace zed_tools {

// =============================================================================
// SVO2FileInfo Methods
// =============================================================================

std::string SVO2FileInfo::getFormattedSize() const {
    return FileUtils::formatFileSize(fileSizeBytes);
}

// =============================================================================
// FileUtils Implementation
// =============================================================================

namespace FileUtils {

std::vector<SVO2FileInfo> scanForSVO2Files(
    const std::string& directoryPath,
    bool recursive
) {
    std::vector<SVO2FileInfo> results;
    
    // Validate directory exists
    if (!directoryExists(directoryPath)) {
        return results; // Return empty vector if directory doesn't exist
    }
    
    fs::path dirPath(directoryPath);
    
    try {
        // Choose iterator based on recursive flag
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
                if (entry.is_regular_file() && validateSVO2File(entry.path())) {
                    SVO2FileInfo info;
                    info.filePath = entry.path();
                    info.fileName = entry.path().filename().string();
                    info.parentFolder = entry.path().parent_path().filename().string();
                    info.fileSizeBytes = getFileSize(entry.path());
                    info.isValidFlightFolder = isFlightFolder(info.parentFolder);
                    
                    results.push_back(info);
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(dirPath)) {
                if (entry.is_regular_file() && validateSVO2File(entry.path())) {
                    SVO2FileInfo info;
                    info.filePath = entry.path();
                    info.fileName = entry.path().filename().string();
                    info.parentFolder = entry.path().parent_path().filename().string();
                    info.fileSizeBytes = getFileSize(entry.path());
                    info.isValidFlightFolder = isFlightFolder(info.parentFolder);
                    
                    results.push_back(info);
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        // Silently handle permission errors, just return what we found
        // In production, you might want to log this
    }
    
    return results;
}

bool validateSVO2File(const fs::path& filePath) {
    // Check 1: File must exist
    if (!fileExists(filePath)) {
        return false;
    }
    
    // Check 2: Must have .svo2 extension (case-insensitive)
    std::string ext = getExtension(filePath);
    if (ext != "svo2") {
        return false;
    }
    
    // Check 3: File size must be > 0
    if (getFileSize(filePath) == 0) {
        return false;
    }
    
    // Check 4: Must be a regular file (not directory or symlink)
    try {
        if (!fs::is_regular_file(filePath)) {
            return false;
        }
    } catch (const fs::filesystem_error&) {
        return false;
    }
    
    return true;
}

bool isFlightFolder(const std::string& folderName) {
    // Pattern: flight_YYYYMMDD_HHMMSS
    // Example: flight_20251107_143056
    std::regex flightPattern(R"(flight_\d{8}_\d{6})");
    return std::regex_match(folderName, flightPattern);
}

std::vector<fs::path> getFlightFolders(
    const std::string& directoryPath,
    bool recursive
) {
    std::vector<fs::path> results;
    
    if (!directoryExists(directoryPath)) {
        return results;
    }
    
    fs::path dirPath(directoryPath);
    
    try {
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
                if (entry.is_directory()) {
                    std::string folderName = entry.path().filename().string();
                    if (isFlightFolder(folderName)) {
                        results.push_back(entry.path());
                    }
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(dirPath)) {
                if (entry.is_directory()) {
                    std::string folderName = entry.path().filename().string();
                    if (isFlightFolder(folderName)) {
                        results.push_back(entry.path());
                    }
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        // Return what we found so far
    }
    
    return results;
}

bool fileExists(const fs::path& filePath) {
    try {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

bool directoryExists(const fs::path& directoryPath) {
    try {
        return fs::exists(directoryPath) && fs::is_directory(directoryPath);
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

uintmax_t getFileSize(const fs::path& filePath) {
    try {
        if (fileExists(filePath)) {
            return fs::file_size(filePath);
        }
    } catch (const fs::filesystem_error&) {
        // Return 0 on error
    }
    return 0;
}

std::string formatFileSize(uintmax_t bytes) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;
    const double TB = GB * 1024.0;
    
    if (bytes >= TB) {
        oss << (bytes / TB) << " TB";
    } else if (bytes >= GB) {
        oss << (bytes / GB) << " GB";
    } else if (bytes >= MB) {
        oss << (bytes / MB) << " MB";
    } else if (bytes >= KB) {
        oss << (bytes / KB) << " KB";
    } else {
        oss << bytes << " bytes";
    }
    
    return oss.str();
}

bool createDirectory(const fs::path& directoryPath) {
    try {
        if (directoryExists(directoryPath)) {
            return true; // Already exists
        }
        return fs::create_directories(directoryPath);
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

fs::path getAbsolutePath(const fs::path& relativePath) {
    try {
        return fs::absolute(relativePath);
    } catch (const fs::filesystem_error&) {
        return relativePath; // Return original on error
    }
}

std::string getExtension(const fs::path& filePath) {
    std::string ext = filePath.extension().string();
    
    // Remove leading dot
    if (!ext.empty() && ext[0] == '.') {
        ext = ext.substr(1);
    }
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); });
    
    return ext;
}

std::string getStem(const fs::path& filePath) {
    return filePath.stem().string();
}

std::string sanitizeFilename(const std::string& filename) {
    std::string result = filename;
    
    // Characters not allowed in Windows filenames: < > : " / \ | ? *
    // Also remove control characters (0-31)
    const std::string invalidChars = "<>:\"/\\|?*";
    
    for (char& c : result) {
        // Replace invalid characters with underscore
        if (invalidChars.find(c) != std::string::npos || c < 32) {
            c = '_';
        }
    }
    
    // Remove leading/trailing spaces and dots (Windows doesn't like them)
    while (!result.empty() && (result.front() == ' ' || result.front() == '.')) {
        result.erase(result.begin());
    }
    while (!result.empty() && (result.back() == ' ' || result.back() == '.')) {
        result.pop_back();
    }
    
    // Ensure result is not empty
    if (result.empty()) {
        result = "unnamed";
    }
    
    return result;
}

} // namespace FileUtils

} // namespace zed_tools
