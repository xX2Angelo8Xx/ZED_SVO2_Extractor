/**
 * @file svo_handler.hpp
 * @brief SVO2 file handler using ZED SDK
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 * 
 * Provides a RAII wrapper around ZED Camera API for SVO2 file operations.
 * Handles opening, reading properties, and safe resource cleanup.
 */

#pragma once

#include <string>
#include <sl/Camera.hpp>
#include "file_utils.hpp"

namespace zed_tools {

/**
 * @brief Structure to hold SVO file properties
 */
struct SVOProperties {
    // Video properties
    int width;                          ///< Video width in pixels
    int height;                         ///< Video height in pixels
    float fps;                          ///< Frames per second
    int totalFrames;                    ///< Total number of frames
    double durationSeconds;             ///< Duration in seconds
    
    // Camera properties
    std::string cameraModel;            ///< ZED camera model (ZED, ZED2, ZED2i, etc.)
    std::string serialNumber;           ///< Camera serial number
    std::string firmwareVersion;        ///< Camera firmware version
    
    // Recording properties
    std::string recordingDateTime;      ///< When recording was made
    sl::RESOLUTION resolution;          ///< ZED resolution enum
    
    /**
     * @brief Get resolution as string
     * @return String like "HD1080", "HD720", "VGA", etc.
     */
    std::string getResolutionString() const;
    
    /**
     * @brief Get camera model as string
     * @return String like "ZED 2i", "ZED 2", "ZED", etc.
     */
    std::string getCameraModelString() const;
};

/**
 * @brief RAII wrapper for ZED Camera SVO file handling
 * 
 * This class manages the lifecycle of a ZED Camera object when reading SVO files.
 * It automatically handles resource cleanup and provides safe access to SVO data.
 * 
 * Example usage:
 * @code
 * SVOHandler handler("path/to/file.svo2");
 * if (handler.open()) {
 *     SVOProperties props = handler.getProperties();
 *     std::cout << "FPS: " << props.fps << std::endl;
 *     std::cout << "Frames: " << props.totalFrames << std::endl;
 *     
 *     while (handler.grab()) {
 *         sl::Mat image;
 *         handler.retrieveImage(image, sl::VIEW::LEFT);
 *         // Process frame...
 *     }
 * }
 * // Automatically closed when handler goes out of scope
 * @endcode
 */
class SVOHandler {
public:
    /**
     * @brief Construct handler with SVO file path
     * @param svoFilePath Path to the .svo2 file
     */
    explicit SVOHandler(const std::string& svoFilePath);
    
    /**
     * @brief Destructor - ensures camera is closed
     */
    ~SVOHandler();
    
    // Disable copy (prevent multiple owners of camera resource)
    SVOHandler(const SVOHandler&) = delete;
    SVOHandler& operator=(const SVOHandler&) = delete;
    
    // Allow move semantics
    SVOHandler(SVOHandler&& other) noexcept;
    SVOHandler& operator=(SVOHandler&& other) noexcept;
    
    /**
     * @brief Open the SVO file
     * @return true if successfully opened, false otherwise
     * 
     * This must be called before any other operations.
     * Validates file exists and is accessible before opening.
     */
    bool open();
    
    /**
     * @brief Close the SVO file and release resources
     * 
     * Called automatically by destructor, but can be called manually
     * for early cleanup.
     */
    void close();
    
    /**
     * @brief Check if SVO file is currently open
     * @return true if open and ready to read
     */
    bool isOpen() const;
    
    /**
     * @brief Get SVO file properties
     * @return SVOProperties structure with all metadata
     * @throws std::runtime_error if file is not open
     */
    SVOProperties getProperties() const;
    
    /**
     * @brief Grab next frame from SVO
     * @return true if frame grabbed successfully, false if end of file
     * 
     * This advances the internal frame counter. Call retrieveImage()
     * after grab() to get the actual frame data.
     */
    bool grab();
    
    /**
     * @brief Retrieve image from last grabbed frame
     * @param image Output image buffer
     * @param view Which camera view (LEFT, RIGHT, LEFT_UNRECTIFIED, etc.)
     * @return sl::ERROR_CODE indicating success or failure
     */
    sl::ERROR_CODE retrieveImage(sl::Mat& image, sl::VIEW view);
    
    /**
     * @brief Retrieve depth map from last grabbed frame
     * @param depth Output depth buffer
     * @param measure Type of depth measure (DEPTH, CONFIDENCE, etc.)
     * @return sl::ERROR_CODE indicating success or failure
     */
    sl::ERROR_CODE retrieveMeasure(sl::Mat& depth, sl::MEASURE measure);
    
    /**
     * @brief Get current frame position
     * @return Current frame index (0-based)
     */
    int getCurrentFramePosition() const;
    
    /**
     * @brief Set frame position (seek)
     * @param frameNumber Frame to seek to (0-based)
     * @return true if seek successful
     */
    bool setFramePosition(int frameNumber);
    
    /**
     * @brief Get total number of frames
     * @return Total frames in SVO file, or 0 if not open
     */
    int getTotalFrames() const;
    
    /**
     * @brief Get last error message
     * @return Human-readable error description
     */
    std::string getLastError() const;
    
    /**
     * @brief Get reference to internal ZED Camera object
     * @return Reference to sl::Camera
     * @warning Advanced use only - direct camera access bypasses safety checks
     */
    sl::Camera& getCamera();

private:
    std::string svoFilePath_;           ///< Path to SVO file
    sl::Camera camera_;                 ///< ZED Camera object
    bool isOpen_;                       ///< Open state flag
    std::string lastError_;             ///< Last error message
    
    /**
     * @brief Set last error message
     * @param error Error description
     */
    void setLastError(const std::string& error);
    
    /**
     * @brief Convert ZED error code to string
     * @param errorCode ZED SDK error code
     * @return Human-readable error message
     */
    std::string errorCodeToString(sl::ERROR_CODE errorCode) const;
};

} // namespace zed_tools
