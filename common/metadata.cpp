/**
 * @file metadata.cpp
 * @brief Implementation of metadata handling
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "metadata.hpp"
#include <filesystem>
#include <regex>
#include <ctime>

namespace fs = std::filesystem;

namespace zed_tools {

// ============================================================================
// FlightInfo Implementation
// ============================================================================

bool FlightInfo::parseFromFolder(const std::string& folderPath) {
    // Extract folder name from path
    fs::path path(folderPath);
    folderName = path.filename().string();
    
    // Parse format: flight_YYYYMMDD_HHMMSS
    std::regex pattern(R"(flight_(\d{8})_(\d{6}))");
    std::smatch matches;
    
    if (std::regex_search(folderName, matches, pattern)) {
        if (matches.size() == 3) {
            std::string dateStr = matches[1].str();  // YYYYMMDD
            std::string timeStr = matches[2].str();  // HHMMSS
            
            // Format date as YYYY-MM-DD
            date = dateStr.substr(0, 4) + "-" + 
                   dateStr.substr(4, 2) + "-" + 
                   dateStr.substr(6, 2);
            
            // Format time as HH:MM:SS
            time = timeStr.substr(0, 2) + ":" + 
                   timeStr.substr(2, 2) + ":" + 
                   timeStr.substr(4, 2);
            
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// JSONBuilder Implementation
// ============================================================================

void JSONBuilder::addIndent() {
    for (int i = 0; i < indent; ++i) {
        ss << "  ";
    }
}

void JSONBuilder::addCommaIfNeeded() {
    if (!firstElement) {
        ss << ",\n";
    } else {
        firstElement = false;
        ss << "\n";
    }
}

void JSONBuilder::beginObject() {
    if (indent > 0) addCommaIfNeeded();
    if (indent > 0) addIndent();
    ss << "{";
    indent++;
    firstElement = true;
}

void JSONBuilder::endObject() {
    indent--;
    ss << "\n";
    addIndent();
    ss << "}";
    firstElement = false;
}

void JSONBuilder::beginArray(const std::string& key) {
    addCommaIfNeeded();
    addIndent();
    ss << "\"" << key << "\": [";
    indent++;
    firstElement = true;
}

void JSONBuilder::endArray() {
    indent--;
    if (!firstElement) {
        ss << "\n";
        addIndent();
    }
    ss << "]";
    firstElement = false;
}

void JSONBuilder::addString(const std::string& key, const std::string& value) {
    addCommaIfNeeded();
    addIndent();
    ss << "\"" << key << "\": \"" << value << "\"";
}

void JSONBuilder::addNumber(const std::string& key, double value) {
    addCommaIfNeeded();
    addIndent();
    ss << "\"" << key << "\": " << value;
}

void JSONBuilder::addNumber(const std::string& key, int value) {
    addCommaIfNeeded();
    addIndent();
    ss << "\"" << key << "\": " << value;
}

void JSONBuilder::addBool(const std::string& key, bool value) {
    addCommaIfNeeded();
    addIndent();
    ss << "\"" << key << "\": " << (value ? "true" : "false");
}

void JSONBuilder::addArrayString(const std::string& value) {
    addCommaIfNeeded();
    addIndent();
    ss << "\"" << value << "\"";
}

// ============================================================================
// VideoMetadata Implementation
// ============================================================================

bool VideoMetadata::saveToJSON(const std::string& outputPath) const {
    JSONBuilder json;
    
    json.beginObject();
    json.addString("type", "video_extraction");
    json.addString("extraction_datetime", extractionDateTime);
    
    // Flight info
    json.addString("folder_name", flightInfo.folderName);
    json.addString("flight_date", flightInfo.date);
    json.addString("flight_time", flightInfo.time);
    json.addString("svo_file", flightInfo.svoFilePath);
    
    // Video properties
    json.addNumber("width", width);
    json.addNumber("height", height);
    json.addNumber("fps", fps);
    json.addNumber("total_frames", totalFrames);
    json.addNumber("duration_seconds", durationSeconds);
    
    // User settings
    json.addString("camera_mode", cameraMode);
    json.addString("video_codec", videoCodec);
    json.addString("output_format", outputFormat);
    
    // Output files
    json.beginArray("output_files");
    for (const auto& file : outputFiles) {
        json.addArrayString(file);
    }
    json.endArray();
    
    json.endObject();
    
    // Write to file
    std::ofstream file(outputPath);
    if (!file.is_open()) return false;
    
    file << json.toString();
    file.close();
    
    return true;
}

// ============================================================================
// FrameMetadata Implementation
// ============================================================================

bool FrameMetadata::saveToJSON(const std::string& outputPath) const {
    JSONBuilder json;
    
    json.beginObject();
    json.addString("type", "frame_extraction");
    json.addString("extraction_datetime", extractionDateTime);
    
    // Flight info
    json.addString("folder_name", flightInfo.folderName);
    json.addString("flight_date", flightInfo.date);
    json.addString("flight_time", flightInfo.time);
    json.addString("svo_file", flightInfo.svoFilePath);
    
    // Video properties
    json.addNumber("width", width);
    json.addNumber("height", height);
    json.addNumber("source_fps", sourceFps);
    json.addNumber("total_source_frames", totalSourceFrames);
    
    // Extraction settings
    json.addString("camera_mode", cameraMode);
    json.addString("image_format", imageFormat);
    json.addNumber("extraction_rate_fps", extractionRate);
    json.addNumber("frame_skip", frameSkip);
    
    // Results
    json.addNumber("total_extracted_frames", totalExtractedFrames);
    json.addNumber("starting_frame_number", startingFrameNumber);
    json.addNumber("ending_frame_number", endingFrameNumber);
    json.addString("output_directory", outputDirectory);
    
    json.endObject();
    
    // Write to file
    std::ofstream file(outputPath);
    if (!file.is_open()) return false;
    
    file << json.toString();
    file.close();
    
    return true;
}

bool FrameMetadata::loadFromJSON(const std::string& inputPath) {
    // Simple JSON parsing for our specific needs
    std::ifstream file(inputPath);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
        // Look for ending_frame_number
        size_t pos = line.find("\"ending_frame_number\"");
        if (pos != std::string::npos) {
            size_t colonPos = line.find(":", pos);
            if (colonPos != std::string::npos) {
                std::string numStr = line.substr(colonPos + 1);
                // Remove whitespace and comma
                numStr.erase(std::remove_if(numStr.begin(), numStr.end(), 
                    [](char c) { return std::isspace(c) || c == ','; }), numStr.end());
                try {
                    startingFrameNumber = std::stoi(numStr) + 1;
                    file.close();
                    return true;
                } catch (...) {
                    break;
                }
            }
        }
    }
    
    file.close();
    return false;
}

// ============================================================================
// DepthMetadata Implementation
// ============================================================================

bool DepthMetadata::saveToJSON(const std::string& outputPath) const {
    JSONBuilder json;
    
    json.beginObject();
    json.addString("type", "depth_analysis");
    json.addString("extraction_datetime", extractionDateTime);
    
    // Flight info
    json.addString("folder_name", flightInfo.folderName);
    json.addString("flight_date", flightInfo.date);
    json.addString("flight_time", flightInfo.time);
    json.addString("svo_file", flightInfo.svoFilePath);
    
    // Video properties
    json.addNumber("width", width);
    json.addNumber("height", height);
    json.addNumber("fps", fps);
    json.addNumber("total_frames", totalFrames);
    
    // Depth settings
    json.addString("neural_mode", neuralMode);
    json.addString("camera_view", cameraView);
    json.addNumber("min_depth_meters", minDepthMeters);
    json.addNumber("max_depth_meters", maxDepthMeters);
    json.addNumber("overlay_transparency", overlayTransparency);
    json.addBool("show_overlay", showOverlay);
    json.addNumber("min_object_pixels", minObjectPixels);
    
    // Statistics
    json.addNumber("min_detected_distance", statistics.minDetectedDistance);
    json.addNumber("max_detected_distance", statistics.maxDetectedDistance);
    json.addNumber("avg_detected_distance", statistics.avgDetectedDistance);
    json.addNumber("total_objects_detected", statistics.totalObjectsDetected);
    json.addNumber("frames_with_detections", statistics.framesWithDetections);
    
    // Output
    json.addString("output_video", outputVideo);
    
    json.endObject();
    
    // Write to file
    std::ofstream file(outputPath);
    if (!file.is_open()) return false;
    
    file << json.toString();
    file.close();
    
    return true;
}

// ============================================================================
// Utility Functions
// ============================================================================

std::string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace zed_tools
