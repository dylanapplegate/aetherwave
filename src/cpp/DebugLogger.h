#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace Aetherwave {

class DebugLogger {
public:
    static DebugLogger& getInstance() {
        static DebugLogger instance;
        return instance;
    }

    void log(const std::string& category, const std::string& message);
    void logWindowEvent(const std::string& event, const std::string& details);
    void logLayoutEvent(const std::string& event, const std::string& details);
    void logDisplayEvent(const std::string& event, const std::string& details);

    void setLogFile(const std::string& filename);
    void enableConsoleOutput(bool enable) { consoleOutput = enable; }

private:
    DebugLogger() : consoleOutput(true) {
        setLogFile("aetherwave_debug.log");
    }

    ~DebugLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    std::string getCurrentTimestamp();
    void writeLog(const std::string& category, const std::string& message);

    std::ofstream logFile;
    bool consoleOutput;

    // Prevent copying
    DebugLogger(const DebugLogger&) = delete;
    DebugLogger& operator=(const DebugLogger&) = delete;
};

// Convenience macros for easier logging
#define DEBUG_LOG(category, message) \
    Aetherwave::DebugLogger::getInstance().log(category, message)

#define DEBUG_WINDOW(event, details) \
    Aetherwave::DebugLogger::getInstance().logWindowEvent(event, details)

#define DEBUG_LAYOUT(event, details) \
    Aetherwave::DebugLogger::getInstance().logLayoutEvent(event, details)

#define DEBUG_DISPLAY(event, details) \
    Aetherwave::DebugLogger::getInstance().logDisplayEvent(event, details)

} // namespace Aetherwave
