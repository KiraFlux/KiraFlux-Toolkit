#pragma once

#include <Arduino.h>
#include <functional>
#include <kf/aliases.hpp>


namespace kf {

/// Тактовая кнопка
struct Button {

    enum class Mode : u8 {
        /// Режим подтягивания вверх
        PullUp,
        /// Режим подтягивания вниз
        PullDown
    };

    enum class PullType : u8 {
        External,
        Internal,
    };

private:

    /// Длительность дребезга в миллисекундах
    static constexpr auto debounce_ms = 50; // todo Задать параметром

public:

    /// Номер пина кнопки
    const u8 pin;
    /// Режим работы кнопки
    const Mode mode;
    /// Обработчик нажатия кнопки
    std::function<void()> handler{nullptr};

private:

    /// Предыдущее состояние кнопки
    bool last_state{false};
    /// Время последнего нажатия
    u32 last_press_ms{0};

public:

    explicit Button(gpio_num_t pin, Mode mode = Mode::PullDown) :
        pin{static_cast<u8>(pin)}, mode{mode} {}

    /// Инициализация кнопки
    inline void init(PullType pull_type) const noexcept {
        pinMode(pin, matchMode(pull_type));
    }

    /// Обновление состояния кнопки
    void poll() noexcept {
        const auto current_state = read();
        const auto now = millis();

        if (current_state and not last_state) {
            if (now - last_press_ms > debounce_ms) {
                if (handler) {
                    handler();
                }
                last_press_ms = now;
            }
        }

        last_state = current_state;
    }

    /// Считать значение кнопки
    [[nodiscard]] bool read() const noexcept {
        if (mode == Mode::PullUp) {
            return not digitalRead(pin);
        }
        return digitalRead(pin);
    }

private:

    [[nodiscard]] inline u8 matchMode(PullType pull_type) const noexcept {
        if (PullType::External == pull_type) {
            return INPUT;
        }
        
        // else - Internal

        if (mode == Mode::PullUp) {
            return INPUT_PULLUP;
        }

        return INPUT_PULLDOWN;
    }
};

} // namespace kf
