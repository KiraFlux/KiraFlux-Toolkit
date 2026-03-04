// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Preferences.h>
#include <type_traits>

namespace kf::memory {

/// @brief Persistent storage wrapper for ESP32 Preferences
/// @tparam T Data type to store
/// @note Uses ESP32's Preferences library for non-volatile storage
template<typename T> struct Storage final {
    static_assert(std::is_trivial_v<T>, "T must be trivially copyable");

private:
    static constexpr const char *preferences_namespace = "kf-cfg";///< Preferences namespace for all KiraFlux configurations

public:
    const char *key;///< Unique key for this storage instance
    T config;       ///< Current configuration data in RAM

    /// @brief Load config from persistent storage (FLASH)
    /// @return true if configuration loaded successfully, false otherwise
    bool load() noexcept {
        Preferences preferences{};
        bool ok{false};

        if (not(ok = preferences.begin(preferences_namespace, true))) { goto ret; }
        if (not(ok = preferences.getBytes(key, &config, sizeof(T)) == sizeof(T))) { goto end; }

    end:
        preferences.end();
    ret:
        return ok;
    }

    /// @brief Save configuration to persistent storage (FLASH)
    /// @return true if saved successfully, false otherwise
    bool save() noexcept {
        Preferences preferences{};
        bool ok{false};

        if (not(ok = preferences.begin(preferences_namespace, false))) { goto ret; }
        if (not(ok = preferences.putBytes(key, &config, sizeof(T)) == sizeof(T))) { goto end; }

    end:
        preferences.end();
    ret:
        return ok;
    }

    /// @brief Erase configuration from persistent storage (FLASH)
    /// @return true if erased successfully, false otherwise
    bool erase() noexcept {
        Preferences preferences{};
        bool ok{false};

        if (not(ok = preferences.begin(preferences_namespace, false))) { goto ret; }
        if (not(ok = preferences.remove(key))) { goto end; }

    end:
        preferences.end();
    ret:
        return ok;
    }

    Storage(const Storage &) = delete;// non-copyable
};

}// namespace kf