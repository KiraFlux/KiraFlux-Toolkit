// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

/// @file    mixin/Configured.hpp
/// @brief   Stores a const reference to a configuration object.

namespace kf::mixin {

struct ConfiguredTag {};

/// @tparam ConfigType The configuration type (stored as a const reference).
template<typename ConfigType> struct Configured : ConfiguredTag {

    /// @brief Constructs a configurable object
    /// @param config Configuration reference that must outlive the object
    explicit constexpr Configured(ConfigType const &config) noexcept :
        _config{config} {}

    /// @brief Returns the stored configuration.
    /// @return Const reference to the configuration.
    [[nodiscard]] constexpr ConfigType const &config() const noexcept {
        return _config;
    }

private:
    ConfigType const &_config;
};

}// namespace kf::mixin