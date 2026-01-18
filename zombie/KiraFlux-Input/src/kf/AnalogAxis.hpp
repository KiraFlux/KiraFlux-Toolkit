#pragma once

#include <esp32-hal-gpio.h>
#include <esp32-hal-adc.h>
#include <kf/tools/filters/ExponentialFilter.hpp>
#include <kf/aliases.hpp>


namespace kf {

/// Джойстик с одной осью
struct AnalogAxis final {

private:

    /// Максимальное аналоговое значение
    static constexpr auto max_analog_value = 4095;

    /// Аналоговый центр по умолчанию (Среднее значение)
    static constexpr auto default_analog_center = max_analog_value / 2;

public:

    /// Ось инвертирована
    bool inverted{false};

private:

    /// Пин подключения джойстика
    const u8 pin;
    /// фильтр значений
    tools::ExponentialFilter<f32> filter;
    i16 range_negative{default_analog_center};
    i16 range_positive{max_analog_value - default_analog_center};

public:

    /// Сырое Значение меньше которого ось считается 0
    i16 dead_zone{0};

    explicit AnalogAxis(gpio_num_t pin, f32 k) noexcept:
        pin{static_cast<u8>(pin)}, filter{k} {}

    /// Инициализировать джойстик
    inline void init() const noexcept {
        pinMode(pin, INPUT);
    }

    /// Обновить значение аналогового цента
    void updateCenter(i16 new_center) noexcept {
        range_negative = new_center;
        range_positive = static_cast<i16>(max_analog_value - new_center);
    }

    /// Считать (сырое) аналоговое значение
    [[nodiscard]] inline int readRaw() const noexcept {
        return analogRead(pin);
    }

    /// Считать нормализованное значение [0.0..1.0]
    [[nodiscard]] f32 read() noexcept {
        return inverted ? -pureRead() : pureRead();
    }

private:

    f32 pureRead() noexcept {
        const auto deviation = readRaw() - range_negative;

        if (std::abs(deviation) < dead_zone) {
            return 0.0f;
        }

        const auto filtered = filter.calc(static_cast<f32>(deviation));

        if (filtered < 0.0f) {
            return filtered / static_cast<f32>(range_negative);
        } else {
            return filtered / static_cast<f32>(range_positive);
        }
    }

};
}