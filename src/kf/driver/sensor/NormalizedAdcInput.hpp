// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/filter/ExponentialFilter.hpp"
#include "kf/gpio.hpp"
#include "kf/math.hpp"
#include "kf/primitives.hpp"
#include "kf/tuner/SampleCollectingTuner.hpp"

#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

#include "kf/driver/sensor/SensorDriver.hpp"

namespace kf::internal {

struct NormalizedAdcInputConfig final {
    using AdcSignedValue = i16;

    bool inverted;
    AdcSignedValue dead_zone, range_positive, range_negative;

    [[nodiscard]] static constexpr AdcSignedValue calcPositiveRange(AdcSignedValue max_analog_value, AdcSignedValue center) noexcept {
        return static_cast<AdcSignedValue>(max_analog_value - center);
    }

    [[nodiscard]] static constexpr AdcSignedValue calcNegativeRange(AdcSignedValue center) noexcept {
        return center;
    }
};

}// namespace kf::internal

namespace kf::driver::sensor {

/// @brief Single analog joystick axis with filtering and dead-zone compensation
struct NormalizedAdcInput final :

    SensorDriver<NormalizedAdcInput, f32, void()>,
    mixin::Configured<internal::NormalizedAdcInputConfig>

{
    using Self = NormalizedAdcInput;

    using FilterImpl = filter::ExponentialFilter<f32>;
    using Config = internal::NormalizedAdcInputConfig;

    /// @brief Tuner for a single analog axis.
    /// @note Inherits from SampleCollectingTuner and implements the concrete logic:
    ///
    ///       - on reset: reset min, max, sum.
    ///
    ///       - on sample: update min, max, sum.
    ///
    ///       - on calculate: compute dead zone and ranges based on collected data.
    struct Tuner final : tuner::SampleCollectingTuner<Tuner, Config> {

        explicit Tuner(Config &config, NormalizedAdcInput &normalized_input, usize samples) :
            tuner::SampleCollectingTuner<Tuner, Config>{config, samples}, _normalized_input{normalized_input} {}

    private:
        NormalizedAdcInput &_normalized_input;
        i64 _sum{};
        Config::AdcSignedValue _max_sample{}, _min_sample{};

        KF_IMPL_SAMPLE_COLLECTING_TUNER(Tuner, Config);

        void resetImpl() noexcept {
            _max_sample = 0;
            _min_sample = gpio::AdcInput::maxValue();
            _sum = 0;
        }

        void pollImpl() noexcept {
            auto const sample = Config::AdcSignedValue(_normalized_input.readRaw());
            _max_sample = math::max(_max_sample, sample);
            _min_sample = math::min(_min_sample, sample);
            _sum += sample;
        }

        void calculateImpl(Config &config) const noexcept {
            constexpr auto margin{10};
            constexpr auto zone_percents{10};

            config.dead_zone = static_cast<Config::AdcSignedValue>((_max_sample - _min_sample) / zone_percents + margin);
            auto const center = static_cast<Config::AdcSignedValue>(_sum / this->samples_total);
            config.range_positive = Config::calcPositiveRange(gpio::AdcInput::maxValue(), center);
            config.range_negative = Config::calcNegativeRange(center);
        }
    };

    explicit NormalizedAdcInput(Config const &config, typename FilterImpl::Config const &filter_config, gpio::GpioNumber gpio_num) noexcept :
        mixin::Configured<Config>{config}, _filter{filter_config}, _adc_input{gpio_num} {}

    [[nodiscard]] gpio::AdcInput::Value readRaw() const noexcept {
        return _adc_input.read();
    }

private:
    FilterImpl _filter;
    gpio::AdcInput _adc_input;

    KF_IMPL_SENSOR_DRIVER(Self, f32, void());

    void initImpl() noexcept {
        _adc_input.init();
    }

    [[nodiscard]] f32 readImpl() noexcept {
        // Applies dead zone, filtering, and optional inversion
        auto const deviation = static_cast<Config::AdcSignedValue>(readRaw()) - this->config().range_negative;

        if (math::abs(deviation) < this->config().dead_zone) {
            return 0.0f;
        }

        auto value = _filter.calc(f32(deviation));

        value /= f32((value < 0.0f) ? this->config().range_negative : this->config().range_positive);

        if (this->config().inverted) {
            value *= -1;
        }

        return value;
    }
};

}// namespace kf::driver::sensor