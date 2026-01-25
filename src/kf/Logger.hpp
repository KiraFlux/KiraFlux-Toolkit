// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/pattern/Singleton.hpp"


namespace kf {

/// @brief Logging system for embedded applications
/// @note Singleton logger with configurable output handler and log levels
struct Logger final : Singleton<Logger> {
    friend struct Singleton<Logger>;

    using WriteHandler = void (*)(StringView);///< Output handler function type

    WriteHandler writer{nullptr};///< Current output handler (nullptr disables logging)

    /// @brief Internal log message formatting and output
    /// @param level Log level string
    /// @param f Source file name
    /// @param format printf-style format string
    /// @param ... Variable arguments matching format specifiers
    void log(const char *level, const char *f, const char *format, ...) const noexcept {
        if (writer == nullptr) { return; }

        char buffer[128];
        size_t pos = 0;

        // Format prefix [timestamp|level|file]
        int prefix_len = snprintf(buffer, sizeof(buffer),
                                  "[%lu|%s|%s] ",
                                  millis(), level, f);

        if (prefix_len > 0) {
            pos = min(static_cast<usize>(prefix_len), sizeof(buffer) - 1);
        }

        // Format main message
        if (pos < sizeof(buffer)) {
            va_list args;
            va_start(args, format);
            int msg_len = vsnprintf(
                buffer + pos,
                sizeof(buffer) - pos,
                format,
                args);
            va_end(args);

            if (msg_len > 0) {
                pos += min(static_cast<size_t>(msg_len), sizeof(buffer) - pos - 1);
            }
        }

        // Add newline
        if (pos < sizeof(buffer) - 1) {
            buffer[pos] = '\n';
            pos++;
        } else {
            // If buffer full, replace last character with newline
            buffer[sizeof(buffer) - 2] = '\n';
            pos = sizeof(buffer) - 1;
        }

        writer({buffer, pos});
    }
};

}// namespace kf

/// @brief Debug log level (lowest severity)
#define kf_Logger_level_debug 0

/// @brief Info log level
#define kf_Logger_level_info 1

/// @brief Warning log level
#define kf_Logger_level_warn 2

/// @brief Error log level
#define kf_Logger_level_error 3

/// @brief Fatal log level (highest severity)
#define kf_Logger_level_fatal 4

/// @brief No logging (disable all output)
#define kf_Logger_level_none 5

/// @brief Current log level (default: debug)
#if not defined(kf_Logger_level)
#define kf_Logger_level kf_Logger_level_debug
#endif

/// @brief Set log output handler function
/// @param func Function pointer accepting Slice<const char>
/// @note Only active when log level is not "none"
#if kf_Logger_level < kf_Logger_level_none
#define kf_Logger_setWriter(func) kf::Logger::instance().writer = func
#else
#define kf_Logger_setWriter(func)
#endif

/// @brief Log debug message (enabled when kf_Logger_level <= debug)
/// @param ... printf-style format string and arguments
#if kf_Logger_level_debug >= kf_Logger_level
#define kf_Logger_debug(...) kf::Logger::instance().log("Debug", __FILE__, __VA_ARGS__)
#else
#define kf_Logger_debug(...)
#endif

/// @brief Log info message (enabled when kf_Logger_level <= info)
/// @param ... printf-style format string and arguments
#if kf_Logger_level_info >= kf_Logger_level
#define kf_Logger_info(...) kf::Logger::instance().log("Info", __FILE__, __VA_ARGS__)
#else
#define kf_Logger_info(...)
#endif

/// @brief Log warning message (enabled when kf_Logger_level <= warn)
/// @param ... printf-style format string and arguments
#if kf_Logger_level_warn >= kf_Logger_level
#define kf_Logger_warn(...) kf::Logger::instance().log("Warn", __FILE__, __VA_ARGS__)
#else
#define kf_Logger_warn(...)
#endif

/// @brief Log error message (enabled when kf_Logger_level <= error)
/// @param ... printf-style format string and arguments
#if kf_Logger_level_error >= kf_Logger_level
#define kf_Logger_error(...) kf::Logger::instance().log("Error", __FILE__, __VA_ARGS__)
#else
#define kf_Logger_error(...)
#endif

/// @brief Log fatal message (enabled when kf_Logger_level <= fatal)
/// @param ... printf-style format string and arguments
#if kf_Logger_level_fatal >= kf_Logger_level
#define kf_Logger_fatal(...) kf::Logger::instance().log("Fatal", __FILE__, __VA_ARGS__)
#else
#define kf_Logger_fatal(...)
#endif