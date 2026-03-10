// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/filters/ExponentialFilter.hpp"
#include "kf/tuner/SampleCollectingTuner.hpp"

namespace kf::input {

/// @brief Single analog joystick axis with filtering and dead-zone compensation
/// @note Uses ESP32's 12-bit ADC (0-4095 range) with configurable filtering
struct AnalogAxis final {
    using AdcSignedValue = i16;

    struct Config {
        using SampleType = AdcSignedValue;

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

        Config(gpio_num_t pin, Mode mode) noexcept :
            pin{static_cast<u8>(pin)}, mode{mode} {}

        [[nodiscard]] constexpr static AdcSignedValue calcPositiveRange(AdcSignedValue center) noexcept {
            return static_cast<AdcSignedValue>(max_analog_value - center);
        }

        [[nodiscard]] constexpr static AdcSignedValue calcNegativeRange(AdcSignedValue center) noexcept {
            return center;
        }
    };

    struct Tuner final : tuner::SampleCollectingTuner<Tuner, Config> {

    private:
        AdcSignedValue _max_sample{};
        AdcSignedValue _min_sample{};
        i64 _sum{};

    public:
        explicit Tuner(Config &config, u16 samples) :
            SampleCollectingTuner{config, samples} {}

    private:
        // SampleCollectingTuner impl

        friend struct SampleCollectingTuner<Tuner, Config>;

        void onStartImpl() noexcept {
            _max_sample = 0;
            _min_sample = Config::max_analog_value;
            _sum = 0;
        }

        void onSampleImpl(AdcSignedValue sample) noexcept {
            _max_sample = kf::max(_max_sample, sample);
            _min_sample = kf::min(_min_sample, sample);
            _sum += sample;
        }

        void calculateImpl(Config &c) const noexcept {
            constexpr auto margin{10};
            constexpr auto zone_percents{10};
            c.dead_zone = static_cast<AdcSignedValue>((_max_sample - _min_sample) / zone_percents + margin);

            const auto center = static_cast<AdcSignedValue>(_sum / samples_total);
            c.range_positive = Config::calcPositiveRange(center);
            c.range_negative = Config::calcNegativeRange(center);
        }
    };

private:
    const Config &_config;
    math::filters::ExponentialFilter<f32> _filter;

public:
    /// @param k Filter coefficient (0.0 to 1.0, higher = more smoothing)
    explicit AnalogAxis(const Config &config, f32 k) noexcept :
        _config{config}, _filter{k} {}

    inline void init() const noexcept {
        pinMode(_config.pin, INPUT);
    }

    [[nodiscard]] inline AdcSignedValue readRaw() const noexcept {
        return static_cast<AdcSignedValue>(analogRead(_config.pin));
    }

    /// @brief Read normalized axis position
    /// @return Filtered value normalized to [-1.0, 1.0] range
    /// @note Applies dead zone, filtering, and optional inversion
    [[nodiscard]] f32 read() noexcept {
        if (_config.mode == Config::Mode::Inverted) {
            return -pureRead();
        } else {
            return pureRead();
        }
    }

private:
    /// @brief Internal normalized reading without inversion
    [[nodiscard]] f32 pureRead() noexcept {
        const auto deviation = readRaw() - _config.range_negative;

        if (kf::abs(deviation) < _config.dead_zone) {
            return 0.0f;
        }

        const auto filtered = _filter.calc(static_cast<f32>(deviation));

        if (filtered < 0.0f) {
            return filtered / static_cast<f32>(_config.range_negative);
        } else {
            return filtered / static_cast<f32>(_config.range_positive);
        }
    }
};

}// namespace kf::input