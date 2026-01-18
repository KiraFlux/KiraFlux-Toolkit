#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"
#include "kf/tools/filters/LowFrequencyFilter.hpp"

namespace kf {
namespace tools {

/// @brief Реализация PID регулятора с внешней зависимостью настроек
struct PID {

public:
    /// @brief Настройки регулятора
    struct Settings {
        /// @brief Коэффициент компонент регулятора
        f32 p, i, d;

        /// @brief Ограничение значение интеграла
        f32 i_limit;

        /// @brief Ограничение выходного значения
        f32 output_limit;
    };

private:
    const Settings &settings;
    LowFrequencyFilter<f32> dx_filter;
    f32 dx{0};
    f32 ix{0};
    f32 last_error{NAN};

public:
    explicit PID(const Settings &settings, f32 dx_filter_alpha = 1.0f) :
        settings{settings}, dx_filter{dx_filter_alpha} {}

    f32 calc(f32 error, f32 dt) {
        constexpr auto max_dt = 0.1f;

        if (dt <= 0.0f or dt > max_dt) {
            return 0.0f;
        }

        if (settings.i != 0.0f) {
            ix += error * dt;
            ix = constrain(ix, -settings.i_limit, settings.i_limit);
        }

        if (settings.d != 0.0f and not isnan(last_error)) {
            dx = dx_filter.calc((error - last_error) / dt);
        } else {
            dx = 0.0f;
        }
        last_error = error;

        const auto output = settings.p * error + settings.i * ix + settings.d * dx;
        return constrain(output, -settings.output_limit, settings.output_limit);
    }

    void reset() {
        dx = 0.0f;
        ix = 0.0f;
        last_error = NAN;
    }
};

}// namespace tools
}// namespace kf
