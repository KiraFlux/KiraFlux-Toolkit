// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct ConfigurableTag {};

template<typename ConfigType> struct Configurable : ConfigurableTag {
    constexpr explicit Configurable(const ConfigType &config) noexcept : _config{config} {}

    [[nodiscard]] constexpr const ConfigType &config() const noexcept { return _config; }

private:
    const ConfigType &_config;
};

}// namespace kf::mixin