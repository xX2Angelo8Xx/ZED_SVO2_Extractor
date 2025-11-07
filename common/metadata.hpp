/**
 * @file metadata.hpp
 * @brief Metadata handling for ZED SVO2 extraction tools
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

namespace zed_tools {

/**
 * @brief Structure to hold flight folder information
 */
struct FlightInfo {
    std::string folderName;          ///< Original folder name (e.g., flight_20251105_141806)
    std::string date;                ///< Extracted date (YYYY-MM-DD)
    std::string time;                ///< Extracted time (HH:MM:SS)
    std::string svoFilePath;         ///< Path to the SVO2 file
    
    /**
     * @brief Parse folder name to extract date and time
     * @param folderPath Path to the flight folder
     * @return true if parsing successful, false otherwise
     */
    bool parseFromFolder(const std::string& folderPath);
};

/**
 * @brief Metadata for video extraction
 */
struct VideoMetadata {
    std::string extractionDateTime;   ///< When extraction was performed
    FlightInfo flightInfo;            ///< Flight information
    
    // Video properties
    int width;                        ///< Video width
    int height;                       ///< Video height
    double fps;                       ///< Frames per second
    int totalFrames;                  ///< Total frames in video
    double durationSeconds;           ///< Duration in seconds
    
    // User settings
    std::string cameraMode;           ///< "left", "right", "both_separate", "both_sidebyside"
    std::string videoCodec;           ///< Video codec used (H.264/H.265)
    std::string outputFormat;         ///< Output format (mp4)
    
    // Output files
    std::vector<std::string> outputFiles;  ///< List of created video files
    
    /**
     * @brief Save metadata to JSON file
     * @param outputPath Path where to save the JSON file
     * @return true if successful
     */
    bool saveToJSON(const std::string& outputPath) const;
};

/**
 * @brief Metadata for frame extraction
 */
struct FrameMetadata {
    std::string extractionDateTime;   ///< When extraction was performed
    FlightInfo flightInfo;            ///< Flight information
    
    // Video properties
    int width;                        ///< Frame width
    int height;                       ///< Frame height
    double sourceFps;                 ///< Original video FPS
    int totalSourceFrames;            ///< Total frames in source
    
    // Extraction settings
    std::string cameraMode;           ///< "left" or "right"
    std::string imageFormat;          ///< Image format (png)
    int extractionRate;               ///< Frames extracted per second (typically 1)
    int frameSkip;                    ///< Skip rate calculated from FPS
    
    // Results
    int totalExtractedFrames;         ///< Number of frames extracted
    int startingFrameNumber;          ///< First frame number used
    int endingFrameNumber;            ///< Last frame number used
    std::string outputDirectory;      ///< Where frames were saved
    
    /**
     * @brief Save metadata to JSON file
     * @param outputPath Path where to save the JSON file
     * @return true if successful
     */
    bool saveToJSON(const std::string& outputPath) const;
    
    /**
     * @brief Load metadata from existing JSON (for continuous numbering)
     * @param inputPath Path to existing JSON file
     * @return true if successful
     */
    bool loadFromJSON(const std::string& inputPath);
};

/**
 * @brief Metadata for depth analysis
 */
struct DepthMetadata {
    std::string extractionDateTime;   ///< When analysis was performed
    FlightInfo flightInfo;            ///< Flight information
    
    // Video properties
    int width;                        ///< Frame width
    int height;                       ///< Frame height
    double fps;                       ///< Frames per second
    int totalFrames;                  ///< Total frames analyzed
    
    // Depth settings
    std::string neuralMode;           ///< "NEURAL" or "NEURAL_PLUS"
    std::string cameraView;           ///< "left" or "right"
    float minDepthMeters;             ///< Minimum depth range (10.0)
    float maxDepthMeters;             ///< Maximum depth range (50.0)
    int overlayTransparency;          ///< Heatmap transparency (0-100)
    bool showOverlay;                 ///< Whether camera overlay is shown
    int minObjectPixels;              ///< Minimum pixels for object detection
    
    // Analysis results
    struct DepthStatistics {
        float minDetectedDistance;    ///< Minimum distance detected
        float maxDetectedDistance;    ///< Maximum distance detected
        float avgDetectedDistance;    ///< Average distance detected
        int totalObjectsDetected;     ///< Total objects detected across all frames
        int framesWithDetections;     ///< Frames that had detections
    } statistics;
    
    std::string outputVideo;          ///< Path to output heatmap video
    
    /**
     * @brief Save metadata to JSON file
     * @param outputPath Path where to save the JSON file
     * @return true if successful
     */
    bool saveToJSON(const std::string& outputPath) const;
};

/**
 * @brief Utility class for creating JSON strings
 */
class JSONBuilder {
public:
    JSONBuilder() : indent(0) {}
    
    void beginObject();
    void endObject();
    void beginArray(const std::string& key);
    void endArray();
    void addString(const std::string& key, const std::string& value);
    void addNumber(const std::string& key, double value);
    void addNumber(const std::string& key, int value);
    void addBool(const std::string& key, bool value);
    void addArrayString(const std::string& value);
    
    std::string toString() const { return ss.str(); }
    
private:
    std::stringstream ss;
    int indent;
    bool firstElement = true;
    
    void addIndent();
    void addCommaIfNeeded();
};

/**
 * @brief Get current date/time as string
 * @return ISO 8601 formatted date/time string
 */
std::string getCurrentDateTime();

/**
 * @brief Utility namespace for metadata operations
 */
namespace metadata_utils {
    /**
     * @brief Get current date/time as string
     * @return ISO 8601 formatted date/time string
     */
    inline std::string getCurrentDateTime() {
        return zed_tools::getCurrentDateTime();
    }
    
    /**
     * @brief Save list of frame metadata to JSON file
     * @param frameList Vector of FrameMetadata structures
     * @param outputPath Path to output JSON file
     * @return true if successful, false otherwise
     */
    bool saveFrameMetadataList(
        const std::vector<FrameMetadata>& frameList,
        const std::string& outputPath
    );
}

} // namespace zed_tools
