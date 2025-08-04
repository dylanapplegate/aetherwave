#include "DebugLogger.h"
#include <iostream>

namespace Aetherwave {

void DebugLogger::setLogFile(const std::string& filename) {
    if (logFile.is_open()) {
        logFile.close();
    }

    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "⚠️ Failed to open debug log file: " << filename << std::endl;
    } else {
        writeLog("SYSTEM", "Debug logging started");
    }
}

void DebugLogger::log(const std::string& category, const std::string& message) {
    writeLog(category, message);
}

void DebugLogger::logWindowEvent(const std::string& event, const std::string& details) {
    std::ostringstream oss;
    oss << event << " | " << details;
    writeLog("WINDOW", oss.str());
}

void DebugLogger::logLayoutEvent(const std::string& event, const std::string& details) {
    std::ostringstream oss;
    oss << event << " | " << details;
    writeLog("LAYOUT", oss.str());
}

void DebugLogger::logDisplayEvent(const std::string& event, const std::string& details) {
    std::ostringstream oss;
    oss << event << " | " << details;
    writeLog("DISPLAY", oss.str());
}

std::string DebugLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void DebugLogger::writeLog(const std::string& category, const std::string& message) {
    std::string timestamp = getCurrentTimestamp();
    std::string logEntry = "[" + timestamp + "] [" + category + "] " + message;

    // Write to file if open
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
        logFile.flush(); // Ensure immediate write
    }

    // Also write to console if enabled
    if (consoleOutput) {
        std::string emoji;
        if (category == "WINDOW") emoji = "🪟";
        else if (category == "LAYOUT") emoji = "🎨";
        else if (category == "DISPLAY") emoji = "📺";
        else if (category == "SYSTEM") emoji = "🔧";
        else emoji = "📋";

        std::cout << emoji << " " << logEntry << std::endl;
    }
}

} // namespace Aetherwave
