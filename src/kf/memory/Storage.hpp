// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Preferences.h>

#include "kf/Logger.hpp"

namespace kf {

/// @brief Persistent storage wrapper for ESP32 Preferences
/// @tparam T Data type to store (must be trivially copyable)
/// @note Uses ESP32's Preferences library for non-volatile storage
template<typename T> struct Storage final {

private:
    static constexpr const char *preferences_namespace = "kf-cfg";///< Preferences namespace for all KiraFlux configurations

public:
    const char *key;///< Unique key for this storage instance
    T settings;     ///< Current settings data in RAM

    /// @brief Load settings from persistent storage (FLASH)
    /// @return true if settings loaded successfully, false otherwise
    /// @note Logs debug and error messages via kf_Logger
    bool load() {
        kf_Logger_debug("Loading storage %s", key);

        Preferences preferences;

        if (not begin(preferences, true)) {
            return false;
        }

        const auto size = preferences.getBytesLength(key);

        if (size != sizeof(T)) {
            preferences.end();
            kf_Logger_error("%s read fail", key);
            return false;
        }

        preferences.getBytes(key, &settings, sizeof(T));
        preferences.end();

        return true;
    }

    /// @brief Save settings to persistent storage (FLASH)
    /// @return true if settings saved successfully, false otherwise
    /// @note Logs debug messages via kf_Logger
    bool save() {
        kf_Logger_debug("Saving storage %s", key);

        Preferences preferences;
        if (not begin(preferences, false)) {
            return false;
        }

        const auto saved = preferences.putBytes(key, &settings, sizeof(T));
        preferences.end();

        return saved == sizeof(T);
    }

    /// @brief Erase settings from persistent storage (FLASH)
    /// @return true if settings erased successfully, false otherwise
    /// @note Logs debug and error messages via kf_Logger
    bool erase() {
        kf_Logger_debug("Saving storage %s", key);

        Preferences preferences;
        if (not begin(preferences, false)) {
            return false;
        }

        if (not preferences.remove(key)) {
            preferences.end();
            kf_Logger_error("key %s remove fail", key);
            return false;
        }

        return true;
    }

private:
    /// @brief Initialize Preferences instance with configured namespace
    /// @param preferences Preferences instance to initialize
    /// @param read_only Open in read-only mode if true
    /// @return true if Preferences opened successfully
    bool begin(Preferences &preferences, bool read_only) const {
        if (preferences.begin(preferences_namespace, read_only)) {
            return true;
        } else {
            kf_Logger_error("%s begin fail", key);
            return false;
        }
    }

public:
    /// @brief Copy constructor is deleted (non-copyable)
    Storage(const Storage &) = delete;
};

}// namespace kf