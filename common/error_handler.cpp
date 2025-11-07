/**
 * @file error_handler.cpp
 * @brief Implementation of error handling and logging system
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "error_handler.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <filesystem>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX  // Prevent Windows.h from defining min/max macros
    #endif
    #include <windows.h>
    #undef ERROR // Windows.h defines ERROR macro
    #define ENABLE_COLOR_OUTPUT
#endif

namespace zed_tools {

// =============================================================================
// Logger Implementation
// =============================================================================

Logger::Logger()
    : mode_(LogMode::CONSOLE_ONLY)
    , minLevel_(LogLevel::INFO)
    , initialized_(false)
{
}

Logger::~Logger() {
    shutdown();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::initialize(
    const std::string& logFilePath,
    LogMode mode,
    LogLevel minLevel
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Close existing file if open
    if (logFile_.is_open()) {
        logFile_.close();
    }
    
    mode_ = mode;
    minLevel_ = minLevel;
    
    // Open log file if needed
    if (mode == LogMode::FILE_ONLY || mode == LogMode::BOTH) {
        if (logFilePath.empty()) {
            std::cerr << "ERROR: Log file path is required for FILE_ONLY or BOTH mode" << std::endl;
            return false;
        }
        
        // Create directory if needed
        std::filesystem::path logPath(logFilePath);
        if (logPath.has_parent_path()) {
            std::filesystem::create_directories(logPath.parent_path());
        }
        
        logFile_.open(logFilePath, std::ios::out | std::ios::app);
        if (!logFile_.is_open()) {
            std::cerr << "ERROR: Failed to open log file: " << logFilePath << std::endl;
            return false;
        }
    }
    
    initialized_ = true;
    
    // Log initialization message
    log(LogLevel::INFO, "Logger initialized", __FILE__, __LINE__);
    
    return true;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        if (logFile_.is_open()) {
            logFile_.close();
        }
        initialized_ = false;
    }
}

void Logger::log(
    LogLevel level,
    const std::string& message,
    const char* file,
    int line
) {
    // Check if we should log this level
    if (level < minLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Format message
    std::string formatted = formatMessage(level, message, file, line);
    
    // Output based on mode
    switch (mode_) {
        case LogMode::CONSOLE_ONLY:
            writeToConsole(formatted, level);
            break;
            
        case LogMode::FILE_ONLY:
            if (logFile_.is_open()) {
                writeToFile(formatted);
            }
            break;
            
        case LogMode::BOTH:
            writeToConsole(formatted, level);
            if (logFile_.is_open()) {
                writeToFile(formatted);
            }
            break;
    }
}

void Logger::setMinLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = level;
}

LogLevel Logger::getMinLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return minLevel_;
}

bool Logger::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.flush();
    }
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ) % 1000;
    
    std::tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &time);
#else
    localtime_r(&time, &timeInfo);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKN ";
    }
}

std::string Logger::formatMessage(
    LogLevel level,
    const std::string& message,
    const char* file,
    int line
) const {
    std::ostringstream oss;
    
    // Timestamp
    oss << "[" << getCurrentTimestamp() << "] ";
    
    // Level
    oss << "[" << logLevelToString(level) << "] ";
    
    // Message
    oss << message;
    
    // Source location (only for DEBUG and ERROR levels)
    if ((level == LogLevel::DEBUG || level == LogLevel::ERROR || level == LogLevel::FATAL) 
        && file != nullptr && line > 0) {
        // Extract just the filename (not full path)
        std::string filename = file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }
        oss << " (" << filename << ":" << line << ")";
    }
    
    return oss.str();
}

void Logger::writeToConsole(const std::string& formattedMessage, LogLevel level) const {
#ifdef ENABLE_COLOR_OUTPUT
    // Windows console color support
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD originalAttributes;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    originalAttributes = csbi.wAttributes;
    
    // Set color based on level
    switch (level) {
        case LogLevel::DEBUG:
            SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY); // Gray
            break;
        case LogLevel::INFO:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Bright green
            break;
        case LogLevel::WARNING:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow
            break;
        case LogLevel::ERROR:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY); // Bright red
            break;
        case LogLevel::FATAL:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Magenta
            break;
    }
    
    std::cout << formattedMessage << std::endl;
    
    // Restore original color
    SetConsoleTextAttribute(hConsole, originalAttributes);
#else
    // No color support - just print
    std::cout << formattedMessage << std::endl;
#endif
}

void Logger::writeToFile(const std::string& formattedMessage) {
    logFile_ << formattedMessage << std::endl;
}

// =============================================================================
// ErrorResult Implementation
// =============================================================================

ErrorResult ErrorResult::success() {
    return ErrorResult{true, "", 0};
}

ErrorResult ErrorResult::failure(const std::string& msg, int code) {
    return ErrorResult{false, msg, code};
}

bool ErrorResult::isSuccess() const {
    return isSuccessful;
}

bool ErrorResult::isFailure() const {
    return !isSuccessful;
}

const std::string& ErrorResult::getMessage() const {
    return message;
}

int ErrorResult::getCode() const {
    return errorCode;
}

} // namespace zed_tools
