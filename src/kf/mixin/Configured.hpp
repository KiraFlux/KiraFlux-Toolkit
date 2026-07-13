// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct ConfiguredTag {};
// TODO: rename to Configured
/// @tparam ConfigType The configuration type (stored as a const reference).
template<typename ConfigType> struct Configured : ConfiguredTag {

    /// @brief Constructs a configurable object
    /// @param config Configuration reference that must outlive the object
    explicit constexpr Configured(const ConfigType &config) noexcept :
        _config{config} {}

    /// @brief Returns the stored configuration.
    /// @return Const reference to the configuration.
    [[nodiscard]] constexpr const ConfigType &config() const noexcept {
        return _config;
    }

private:
    const ConfigType &_config;
};

}// namespace kf::mixin