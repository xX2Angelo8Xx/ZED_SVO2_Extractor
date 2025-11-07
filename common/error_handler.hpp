/**
 * @file error_handler.hpp
 * @brief Error handling and logging system
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 * 
 * Provides centralized error handling and logging functionality.
 * Features:
 * - Multiple severity levels (DEBUG, INFO, WARNING, ERROR, FATAL)
 * - Console and file output
 * - Timestamp formatting
 * - Thread-safe logging
 * - Compile-time debug/release mode switching
 */

#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <sstream>

namespace zed_tools {

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    DEBUG,      ///< Detailed information for debugging
    INFO,       ///< General informational messages
    WARNING,    ///< Warning messages (non-critical issues)
    ERROR,      ///< Error messages (recoverable errors)
    FATAL       ///< Fatal errors (application should terminate)
};

/**
 * @brief Log output mode
 */
enum class LogMode {
    CONSOLE_ONLY,   ///< Output to console only
    FILE_ONLY,      ///< Output to file only
    BOTH            ///< Output to both console and file
};

/**
 * @brief Singleton logger class for centralized logging
 * 
 * Thread-safe logger that supports multiple output modes and severity levels.
 * In Release builds, DEBUG messages are automatically filtered out.
 * 
 * Example usage:
 * @code
 * // Initialize logger (call once at startup)
 * Logger::getInstance().initialize("app.log", LogMode::BOTH, LogLevel::INFO);
 * 
 * // Log messages
 * LOG_INFO("Application started");
 * LOG_WARNING("Configuration file not found, using defaults");
 * LOG_ERROR("Failed to open file: " + filename);
 * 
 * // Only in Debug builds:
 * LOG_DEBUG("Processing frame " + std::to_string(frameNum));
 * @endcode
 */
class Logger {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to the logger instance
     */
    static Logger& getInstance();
    
    // Disable copy and move
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    /**
     * @brief Initialize the logger
     * @param logFilePath Path to log file (empty for console only)
     * @param mode Output mode (console, file, or both)
     * @param minLevel Minimum severity level to log
     * @return true if initialization successful
     */
    bool initialize(
        const std::string& logFilePath = "",
        LogMode mode = LogMode::CONSOLE_ONLY,
        LogLevel minLevel = LogLevel::INFO
    );
    
    /**
     * @brief Shut down the logger and close files
     */
    void shutdown();
    
    /**
     * @brief Log a message with specified severity
     * @param level Severity level
     * @param message Message to log
     * @param file Source file (use __FILE__)
     * @param line Source line (use __LINE__)
     */
    void log(
        LogLevel level,
        const std::string& message,
        const char* file = nullptr,
        int line = 0
    );
    
    /**
     * @brief Set minimum log level
     * @param level New minimum level
     */
    void setMinLevel(LogLevel level);
    
    /**
     * @brief Get current minimum log level
     * @return Current minimum level
     */
    LogLevel getMinLevel() const;
    
    /**
     * @brief Check if logger is initialized
     * @return true if initialized
     */
    bool isInitialized() const;
    
    /**
     * @brief Flush log buffer (force write to file)
     */
    void flush();

private:
    Logger();
    ~Logger();
    
    mutable std::mutex mutex_;          ///< Thread safety mutex (mutable for const methods)
    std::ofstream logFile_;             ///< Log file stream
    LogMode mode_;                      ///< Output mode
    LogLevel minLevel_;                 ///< Minimum log level
    bool initialized_;                  ///< Initialization state
    
    /**
     * @brief Get current timestamp string
     * @return Formatted timestamp (YYYY-MM-DD HH:MM:SS)
     */
    std::string getCurrentTimestamp() const;
    
    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return String representation
     */
    std::string logLevelToString(LogLevel level) const;
    
    /**
     * @brief Format log message
     * @param level Severity level
     * @param message Message content
     * @param file Source file
     * @param line Source line
     * @return Formatted log entry
     */
    std::string formatMessage(
        LogLevel level,
        const std::string& message,
        const char* file,
        int line
    ) const;
    
    /**
     * @brief Write message to console
     * @param formattedMessage Formatted log message
     * @param level Severity level (for color coding)
     */
    void writeToConsole(const std::string& formattedMessage, LogLevel level) const;
    
    /**
     * @brief Write message to file
     * @param formattedMessage Formatted log message
     */
    void writeToFile(const std::string& formattedMessage);
};

// =============================================================================
// Convenience Macros
// =============================================================================

/**
 * @brief Log debug message (only in Debug builds)
 */
#ifdef _DEBUG
    #define LOG_DEBUG(msg) \
        do { \
            std::ostringstream oss; \
            oss << msg; \
            zed_tools::Logger::getInstance().log( \
                zed_tools::LogLevel::DEBUG, oss.str(), __FILE__, __LINE__ \
            ); \
        } while(0)
#else
    #define LOG_DEBUG(msg) ((void)0)
#endif

/**
 * @brief Log informational message
 */
#define LOG_INFO(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        zed_tools::Logger::getInstance().log( \
            zed_tools::LogLevel::INFO, oss.str(), __FILE__, __LINE__ \
        ); \
    } while(0)

/**
 * @brief Log warning message
 */
#define LOG_WARNING(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        zed_tools::Logger::getInstance().log( \
            zed_tools::LogLevel::WARNING, oss.str(), __FILE__, __LINE__ \
        ); \
    } while(0)

/**
 * @brief Log error message
 */
#define LOG_ERROR(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        zed_tools::Logger::getInstance().log( \
            zed_tools::LogLevel::ERROR, oss.str(), __FILE__, __LINE__ \
        ); \
    } while(0)

/**
 * @brief Log fatal error message
 */
#define LOG_FATAL(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        zed_tools::Logger::getInstance().log( \
            zed_tools::LogLevel::FATAL, oss.str(), __FILE__, __LINE__ \
        ); \
    } while(0)

// =============================================================================
// Error Result Structure
// =============================================================================

/**
 * @brief Result structure for operations that can fail
 * 
 * Provides a clean way to return success/failure with error messages.
 * 
 * Example usage:
 * @code
 * ErrorResult loadFile(const std::string& path) {
 *     if (!fileExists(path)) {
 *         return ErrorResult::failure("File not found: " + path);
 *     }
 *     // ... load file ...
 *     return ErrorResult::success();
 * }
 * 
 * auto result = loadFile("data.txt");
 * if (!result.isSuccess()) {
 *     LOG_ERROR(result.getMessage());
 *     return;
 * }
 * @endcode
 */
struct ErrorResult {
    bool isSuccessful;      ///< Success flag
    std::string message;    ///< Error message (empty if success)
    int errorCode;          ///< Optional error code
    
    /**
     * @brief Create success result
     * @return Success result
     */
    static ErrorResult success();
    
    /**
     * @brief Create failure result
     * @param msg Error message
     * @param code Optional error code
     * @return Failure result
     */
    static ErrorResult failure(const std::string& msg, int code = -1);
    
    /**
     * @brief Check if result is success
     * @return true if successful
     */
    bool isSuccess() const;
    
    /**
     * @brief Check if result is failure
     * @return true if failed
     */
    bool isFailure() const;
    
    /**
     * @brief Get error message
     * @return Error message (empty if success)
     */
    const std::string& getMessage() const;
    
    /**
     * @brief Get error code
     * @return Error code (0 if success)
     */
    int getCode() const;
};

} // namespace zed_tools
