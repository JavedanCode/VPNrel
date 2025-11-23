#pragma once
#include <string>
#include <iostream>

/**
 * @enum LogLevel
 * @brief Defines severity levels for logging.
 */
enum class LogLevel { Info, Warn, Error, Debug };

/**
 * @class Logger
 * @brief Provides a simple logging interface for console and file output.
 *
 * This class centralizes all logging functionality for the VPN system.
 *
 * Step Plan:
 *   Step 1 → Implement basic console logging.
 *   Step 2 → Add timestamps to messages.
 *   Step 3 → Add optional file logging.
 *   Step 4 → Add color-coded output (optional).
 */
class Logger {
public:
    // Logs a message with a given severity level
    static void log(LogLevel level, const std::string& msg);
};
