// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"
#include "kf/algorithm.hpp"
#include "kf/core/attributes.hpp"
#include "kf/math/filters/ExponentialFilter.hpp"
#include "kf/Tuner.hpp"


namespace kf {

/// @brief Single analog joystick axis with filtering and dead-zone compensation
/// @note Uses ESP32's 12-bit ADC (0-4095 range) with configurable filtering
struct AnalogAxis final {
    using AdcSignedValue = i16;

    struct Config {
        using tuned_type = AdcSignedValue;

        static constexpr unsigned adc_bits{12};

        static constexpr AdcSignedValue max_analog_value{(1 << adc_bits) - 1};
        static constexpr AdcSignedValue default_analog_center{max_analog_value / 2};

        enum class Mode : u8 {
            Normal,
            Inverted,
        } mode;
        u8 pin;
        AdcSignedValue dead_zone{0};
        AdcSignedValue range_positive{calcPositiveRange(default_analog_center)};
        AdcSignedValue range_negative{calcNegativeRange(default_analog_center)};

        Config(gpio_num_t pin, Mode mode) noexcept:
            pin{static_cast<u8>(pin)}, mode{mode} {}

        kf_nodiscard constexpr static AdcSignedValue calcPositiveRange(AdcSignedValue center) noexcept {
            return static_cast<AdcSignedValue>(max_analog_value - center);
        }

        kf_nodiscard constexpr static AdcSignedValue calcNegativeRange(AdcSignedValue center) noexcept {
            return center;
        }
    };

    struct AxisTuner final : Tuner<AxisTuner, Config> {
    private:
        AdcSignedValue max_sample{};
        AdcSignedValue min_sample{};
        i64 sum{};

    public:

        explicit AxisTuner(Config &config, u16 samples) :
            Tuner{config, samples} {}

        void onStart() noexcept {
            max_sample = 0;
            min_sample = Config::max_analog_value;
            sum = 0;
        }

        void onSample(AdcSignedValue sample) noexcept {
            max_sample = kf::max(max_sample, sample);
            min_sample = kf::min(min_sample, sample);
            sum += sample;
        }

        void calculate(Config &c) const noexcept {
            constexpr auto margin{10};
            constexpr auto zone_percents{10};
            c.dead_zone = static_cast<AdcSignedValue>((max_sample - min_sample) / zone_percents + margin);

            const auto center = static_cast<AdcSignedValue>(sum / samples_total);
            c.range_positive = Config::calcPositiveRange(center);
            c.range_negative = Config::calcNegativeRange(center);
        }
    };

private:
    const Config &config;
    ExponentialFilter<f32> filter;

public:

    /// @param k Filter coefficient (0.0 to 1.0, higher = more smoothing)
    explicit AnalogAxis(const Config &config, f32 k) noexcept:
        config{config}, filter{k} {}

    inline void init() const noexcept {
        pinMode(config.pin, INPUT);
    }

    kf_nodiscard inline AdcSignedValue readRaw() const noexcept {
        return static_cast<AdcSignedValue>(analogRead(config.pin));
    }

    /// @brief Read normalized axis position
    /// @return Filtered value normalized to [-1.0, 1.0] range
    /// @note Applies dead zone, filtering, and optional inversion
    kf_nodiscard f32 read() noexcept {
        if (config.mode == Config::Mode::Inverted) {
            return -pureRead();
        } else {
            return pureRead();
        }
    }

private:
    /// @brief Internal normalized reading without inversion
    kf_nodiscard f32 pureRead() noexcept {
        const auto deviation = readRaw() - config.range_negative;

        if (kf::abs(deviation) < config.dead_zone) {
            return 0.0f;
        }

        const auto filtered = filter.calc(static_cast<f32>(deviation));

        if (filtered < 0.0f) {
            return filtered / static_cast<f32>(config.range_negative);
        } else {
            return filtered / static_cast<f32>(config.range_positive);
        }
    }
};

}// namespace kf