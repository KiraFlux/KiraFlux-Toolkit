// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/gpio/GPIO.hpp"
#include "kf/math/filters/ExponentialFilter.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/tuner/SampleCollectingTuner.hpp"

namespace kf::drivers::sensors {

/// @brief Single analog joystick axis with filtering and dead-zone compensation
template<typename I> struct NormalizedAdcInput final : mixin::Initable<NormalizedAdcInput<I>, void> {
    kf_crtp_check(I, kf::gpio::AdcInputTag);

    using AdcPinImpl = I;
    using AdcSignedValue = i16;
    using FilterImpl = math::filters::ExponentialFilter<f32>;

    struct Config;// forward declaration

    /// @brief Tuner for a single analog axis.
    /// @note Inherits from SampleCollectingTuner and implements the concrete logic:
    ///
    ///       - on reset: reset min, max, sum.
    ///
    ///       - on sample: update min, max, sum.
    ///
    ///       - on calculate: compute dead zone and ranges based on collected data.
    struct Tuner final : kf::tuner::SampleCollectingTuner<Tuner, Config> {
        using TunerBase = kf::tuner::SampleCollectingTuner<Tuner, Config>;

        explicit Tuner(Config &config, NormalizedAdcInput &normalized_input, u16 samples) :
            _normalized_input{normalized_input}, TunerBase{config, samples} {}

    private:
        NormalizedAdcInput &_normalized_input;
        i64 _sum{};
        AdcSignedValue _max_sample{};
        AdcSignedValue _min_sample{};

        // impl

        friend struct kf::mixin::Resettable<Tuner>;

        void resetImpl() noexcept {
            _max_sample = 0;
            _min_sample = AdcPinImpl::maxValue();
            _sum = 0;
        }

        friend struct kf::mixin::Pollable<Tuner>;

        void pollImpl() noexcept {
            const auto sample = AdcSignedValue(_normalized_input.readRaw());
            _max_sample = kf::max(_max_sample, sample);
            _min_sample = kf::min(_min_sample, sample);
            _sum += sample;
        }

        friend TunerBase;

        void calculateImpl(Config &config) const noexcept {
            constexpr auto margin{10};
            constexpr auto zone_percents{10};

            config.dead_zone = static_cast<AdcSignedValue>((_max_sample - _min_sample) / zone_percents + margin);
            const auto center = static_cast<AdcSignedValue>(_sum / this->samples_total);
            config.range_positive = Config::calcPositiveRange(AdcPinImpl::maxValue(), center);
            config.range_negative = Config::calcNegativeRange(center);
        }
    };

    struct Config {
        bool inverted;
        AdcSignedValue dead_zone;
        AdcSignedValue range_positive;
        AdcSignedValue range_negative;

        [[nodiscard]] Tuner createTuner(NormalizedAdcInput &axis, u16 samples) noexcept {
            return Tuner{*this, axis, samples};
        }

        [[nodiscard]] constexpr static AdcSignedValue calcPositiveRange(AdcSignedValue max_analog_value, AdcSignedValue center) noexcept {
            return static_cast<AdcSignedValue>(max_analog_value - center);
        }

        [[nodiscard]] constexpr static AdcSignedValue calcNegativeRange(AdcSignedValue center) noexcept {
            return center;
        }
    };

    explicit NormalizedAdcInput(const Config &config, const typename FilterImpl::Config &filter_config, AdcPinImpl &&pin) noexcept :
        _config{config}, _filter{filter_config}, _pin{pin} {}

    [[nodiscard]] u16 readRaw() const noexcept { return _pin.read(); }

    /// @brief Read normalized axis position
    /// @return Filtered value normalized to [-1.0, 1.0] range
    /// @note Applies dead zone, filtering, and optional inversion
    [[nodiscard]] f32 read() noexcept {
        auto result = pureRead();

        if (_config.inverted) {
            result *= -1;
        }

        return result;
    }

private:
    const Config &_config;
    FilterImpl _filter;
    AdcPinImpl _pin;

    /// @brief Internal normalized reading without inversion
    [[nodiscard]] f32 pureRead() noexcept {
        const auto deviation = static_cast<AdcSignedValue>(readRaw()) - _config.range_negative;

        if (kf::abs(deviation) < _config.dead_zone) {
            return 0.0f;
        }

        const auto filtered = _filter.calc(f32(deviation));

        if (filtered < 0.0f) {
            return filtered / f32(_config.range_negative);
        } else {
            return filtered / f32(_config.range_positive);
        }
    }

    // Initable impl
    friend struct kf::mixin::Initable<NormalizedAdcInput<I>, void>;

    void initImpl() noexcept {
        _pin.init();
    }
};

}// namespace kf::drivers::sensors