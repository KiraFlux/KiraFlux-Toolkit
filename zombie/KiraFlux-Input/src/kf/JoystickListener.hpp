#pragma once

#include <cstdint>
#include <kf/aliases.hpp>


namespace kf {

/// Прослушивает изменения джойстика
struct JoystickListener {

private:

    /// Порог срабатывания по умолчанию
    static constexpr auto default_threshold = 0.6;

public:

    /// Событие джойстика (Срабатывают однократно)
    enum class Direction : u8 {
        /// Джойстик переместился исходную позицию
        Home = 0x00,

        /// Джойстик переместился вверх
        Up = 0x10,

        /// Джойстик удерживается вверх
        Down = 0x20,

        /// Джойстик удерживается в вниз
        Left = 0x30,

        /// Джойстик удерживается в влево
        Right = 0x40,
    };

    /// Обработчик событий джойстика
    std::function<void(Direction)> handler{nullptr};

    /// Порог срабатывания
    const float threshold{default_threshold};

private:

    /// Предыдущее направление
    Direction last_direction{Direction::Home};

    /// Прослушиваемый джойстик
    Joystick &joystick;

public:

    explicit JoystickListener(Joystick &joy) :
        joystick{joy} {}

    /// Пул обновлений событий джойстика
    void poll() {
        if (not handler) { return; }

        const auto current_direction = getCurrentDirection();

        if (current_direction != last_direction) {
            handler(current_direction);
            last_direction = current_direction;
        }
    }

private:

    Direction getCurrentDirection() {
        const auto x = joystick.axis_x.read();
        const auto y = joystick.axis_y.read();
        const auto abs_x = std::abs(x);
        const auto abs_y = std::abs(y);

        if (abs_x < threshold and abs_y < threshold) {
            return Direction::Home;
        }

        if (abs_x > abs_y) {
            return x > 0 ? Direction::Right : Direction::Left;
        } else {
            return y > 0 ? Direction::Up : Direction::Down;
        }
    }

};

}