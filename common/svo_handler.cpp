/**
 * @file svo_handler.cpp
 * @brief Implementation of SVO2 file handler
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "svo_handler.hpp"
#include <sstream>
#include <iomanip>

namespace zed_tools {

// =============================================================================
// SVOProperties Methods
// =============================================================================

std::string SVOProperties::getResolutionString() const {
    switch (resolution) {
        case sl::RESOLUTION::HD2K:   return "HD2K (2208x1242)";
        case sl::RESOLUTION::HD1080: return "HD1080 (1920x1080)";
        case sl::RESOLUTION::HD720:  return "HD720 (1280x720)";
        case sl::RESOLUTION::VGA:    return "VGA (672x376)";
        default:                     return "Unknown";
    }
}

std::string SVOProperties::getCameraModelString() const {
    return cameraModel;
}

// =============================================================================
// SVOHandler Implementation
// =============================================================================

SVOHandler::SVOHandler(const std::string& svoFilePath)
    : svoFilePath_(svoFilePath)
    , isOpen_(false)
    , lastError_("")
{
}

SVOHandler::~SVOHandler() {
    close();
}

SVOHandler::SVOHandler(SVOHandler&& other) noexcept
    : svoFilePath_(std::move(other.svoFilePath_))
    , isOpen_(other.isOpen_)
    , lastError_(std::move(other.lastError_))
{
    other.isOpen_ = false;
    // Note: sl::Camera cannot be moved, new handler would need to reopen
}

SVOHandler& SVOHandler::operator=(SVOHandler&& other) noexcept {
    if (this != &other) {
        close(); // Close current resources
        
        svoFilePath_ = std::move(other.svoFilePath_);
        isOpen_ = other.isOpen_;
        lastError_ = std::move(other.lastError_);
        
        other.isOpen_ = false;
        // Note: sl::Camera cannot be moved, would need to reopen
    }
    return *this;
}

bool SVOHandler::open() {
    // Check if already open
    if (isOpen_) {
        setLastError("SVO file is already open");
        return false;
    }
    
    // Validate file exists
    if (!FileUtils::validateSVO2File(svoFilePath_)) {
        setLastError("Invalid or non-existent SVO2 file: " + svoFilePath_);
        return false;
    }
    
    // Configure initialization parameters
    sl::InitParameters initParams;
    initParams.input.setFromSVOFile(svoFilePath_.c_str());
    initParams.coordinate_units = sl::UNIT::METER;
    initParams.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP;
    
    // Disable real-time mode for SVO playback (process as fast as possible)
    initParams.svo_real_time_mode = false;
    
    // Open camera with SVO file
    sl::ERROR_CODE err = camera_.open(initParams);
    
    if (err != sl::ERROR_CODE::SUCCESS) {
        setLastError("Failed to open SVO file: " + errorCodeToString(err));
        return false;
    }
    
    isOpen_ = true;
    lastError_.clear();
    return true;
}

void SVOHandler::close() {
    if (isOpen_) {
        camera_.close();
        isOpen_ = false;
    }
}

bool SVOHandler::isOpen() const {
    return isOpen_;
}

SVOProperties SVOHandler::getProperties() const {
    if (!isOpen_) {
        throw std::runtime_error("Cannot get properties: SVO file is not open");
    }
    
    SVOProperties props;
    
    // Get camera information (cast away const since ZED SDK doesn't provide const version)
    sl::CameraInformation camInfo = const_cast<sl::Camera&>(camera_).getCameraInformation();
    
    // Video properties
    sl::Resolution resolution = camInfo.camera_configuration.resolution;
    props.width = resolution.width;
    props.height = resolution.height;
    props.fps = camInfo.camera_configuration.fps;
    
    // Store resolution enum (map from resolution object to enum)
    if (resolution.width == 2208 && resolution.height == 1242) {
        props.resolution = sl::RESOLUTION::HD2K;
    } else if (resolution.width == 1920 && resolution.height == 1080) {
        props.resolution = sl::RESOLUTION::HD1080;
    } else if (resolution.width == 1280 && resolution.height == 720) {
        props.resolution = sl::RESOLUTION::HD720;
    } else if (resolution.width == 672 && resolution.height == 376) {
        props.resolution = sl::RESOLUTION::VGA;
    } else {
        props.resolution = sl::RESOLUTION::HD1080; // Default
    }
    
    // Total frames from SVO (cast away const)
    int totalFrames = const_cast<sl::Camera&>(camera_).getSVONumberOfFrames();
    props.totalFrames = totalFrames;
    props.durationSeconds = (totalFrames > 0 && props.fps > 0) 
                           ? totalFrames / props.fps 
                           : 0.0;
    
    // Camera model
    switch (camInfo.camera_model) {
        case sl::MODEL::ZED:
            props.cameraModel = "ZED";
            break;
        case sl::MODEL::ZED_M:
            props.cameraModel = "ZED Mini";
            break;
        case sl::MODEL::ZED2:
            props.cameraModel = "ZED 2";
            break;
        case sl::MODEL::ZED2i:
            props.cameraModel = "ZED 2i";
            break;
        case sl::MODEL::ZED_X:
            props.cameraModel = "ZED X";
            break;
        case sl::MODEL::ZED_XM:
            props.cameraModel = "ZED X Mini";
            break;
        default:
            props.cameraModel = "Unknown";
            break;
    }
    
    // Serial number
    props.serialNumber = std::to_string(camInfo.serial_number);
    
    // Firmware version
    props.firmwareVersion = std::to_string(camInfo.camera_configuration.firmware_version);
    
    // Recording timestamp (if available in SVO metadata)
    // Note: This might not be available in all SVO files
    props.recordingDateTime = "N/A";
    
    return props;
}

bool SVOHandler::grab() {
    if (!isOpen_) {
        setLastError("Cannot grab: SVO file is not open");
        return false;
    }
    
    sl::ERROR_CODE err = camera_.grab();
    
    if (err == sl::ERROR_CODE::END_OF_SVOFILE_REACHED) {
        return false; // End of file reached (not an error)
    }
    
    if (err != sl::ERROR_CODE::SUCCESS) {
        setLastError("Grab failed: " + errorCodeToString(err));
        return false;
    }
    
    return true;
}

sl::ERROR_CODE SVOHandler::retrieveImage(sl::Mat& image, sl::VIEW view) {
    if (!isOpen_) {
        return sl::ERROR_CODE::CAMERA_NOT_INITIALIZED;
    }
    
    return camera_.retrieveImage(image, view);
}

sl::ERROR_CODE SVOHandler::retrieveMeasure(sl::Mat& depth, sl::MEASURE measure) {
    if (!isOpen_) {
        return sl::ERROR_CODE::CAMERA_NOT_INITIALIZED;
    }
    
    return camera_.retrieveMeasure(depth, measure);
}

int SVOHandler::getCurrentFramePosition() const {
    if (!isOpen_) {
        return -1;
    }
    
    // Cast away const since ZED SDK doesn't provide const version
    return const_cast<sl::Camera&>(camera_).getSVOPosition();
}

bool SVOHandler::setFramePosition(int frameNumber) {
    if (!isOpen_) {
        setLastError("Cannot seek: SVO file is not open");
        return false;
    }
    
    int totalFrames = getTotalFrames();
    if (frameNumber < 0 || frameNumber >= totalFrames) {
        setLastError("Frame number out of range: " + std::to_string(frameNumber));
        return false;
    }
    
    camera_.setSVOPosition(frameNumber);
    return true;
}

int SVOHandler::getTotalFrames() const {
    if (!isOpen_) {
        return 0;
    }
    
    // Cast away const since ZED SDK doesn't provide const version
    return const_cast<sl::Camera&>(camera_).getSVONumberOfFrames();
}

std::string SVOHandler::getLastError() const {
    return lastError_;
}

sl::Camera& SVOHandler::getCamera() {
    return camera_;
}

void SVOHandler::setLastError(const std::string& error) {
    lastError_ = error;
}

std::string SVOHandler::errorCodeToString(sl::ERROR_CODE errorCode) const {
    // Use ZED SDK's built-in error code to string conversion
    return std::string(sl::toString(errorCode).c_str());
}

} // namespace zed_tools
